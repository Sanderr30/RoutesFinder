#pragma once

#include <functional>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>


class AsyncInputHandler {
public:
    using InputCallback = std::function<void(const std::string& input)>;

public:
    AsyncInputHandler(boost::asio::io_context& io_context);

    ~AsyncInputHandler();

    void StartReading(InputCallback callback);

    void StopReading();

    void ShowPrompt(const std::string& prompt = "> ");

    void PrintMessage(const std::string& message);

private:
    void ReadInputLoop();
    void ProcessInput();

private:
    boost::asio::io_context& io_context_;
    InputCallback input_callback_;

    std::thread input_thread_;
    std::queue<std::string> input_queue_;
    std::mutex input_mutex_;
    std::condition_variable input_cv_;

    std::atomic<bool> reading_;
    std::atomic<bool> should_stop_;

    std::unique_ptr<boost::asio::steady_timer> idle_timer_;
    void ScheduleIdleTask();
};
