#include <iostream>
#include <chrono>

#include "input_handler.h"


AsyncInputHandler::AsyncInputHandler(boost::asio::io_context& io_context)
    : io_context_(io_context)
    , reading_(false)
    , should_stop_(false)
{
    idle_timer_ = std::make_unique<boost::asio::steady_timer>(io_context_);
    ScheduleIdleTask();
}


AsyncInputHandler::~AsyncInputHandler() {
    StopReading();
}


void AsyncInputHandler::StartReading(InputCallback callback) {
    if (reading_.load()) {
        return;
    }
    input_callback_ = callback;
    reading_ = true;
    should_stop_ = false;
    input_thread_ = std::thread(&AsyncInputHandler::ReadInputLoop, this);
}


void AsyncInputHandler::StopReading() {
    should_stop_ = true;
    reading_ = false;

    if (input_thread_.joinable()) {
        input_cv_.notify_all();
        input_thread_.join();
    }

    if (idle_timer_) {
        idle_timer_->cancel();
    }
}


void AsyncInputHandler::ShowPrompt(const std::string& prompt) {
    std::cout << prompt << std::flush;
}


void AsyncInputHandler::PrintMessage(const std::string& message) {
    post(io_context_, [message]()
        {
            std::cout << message << std::endl;
            std::cout << "> " << std::flush;
        }
    );
}


void AsyncInputHandler::ReadInputLoop() {
    ShowPrompt();

    while (!should_stop_.load()) {
        std::string input;
        if (std::getline(std::cin, input)) {
            if (!input.empty()) {
                {
                    std::lock_guard<std::mutex> lock(input_mutex_);
                    input_queue_.push(input);
                }

                post(io_context_, [this]()
                    {
                        ProcessInput();
                    }
                );
            }

            if (reading_.load()) {
                ShowPrompt();
            }
        } else {
            break;
        }
    }
}


void AsyncInputHandler::ProcessInput() {
    std::string input;
    {
        std::lock_guard<std::mutex> lock(input_mutex_);
        if (input_queue_.empty()) {
            return;
        }
        input = input_queue_.front();
        input_queue_.pop();
    }

    if (input_callback_ && reading_.load()) {
        input_callback_(input);
    }
}


void AsyncInputHandler::ScheduleIdleTask() {
    if (!idle_timer_) {
        return;
    }

    idle_timer_->expires_after(std::chrono::milliseconds(100));

    idle_timer_->async_wait([this](const boost::system::error_code& ec)
        {
            if (!ec && !should_stop_.load()) {
                ScheduleIdleTask();
            }
        }
    );
}
