#include "task_scheduler.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>


TaskScheduler::TaskScheduler(size_t thread_pool_size)
    : work_guard_(std::make_unique<boost::asio::io_context::work>(io_context_))
    , thread_pool_(std::make_unique<boost::asio::thread_pool>(thread_pool_size))
{}


TaskScheduler::~TaskScheduler() {
    Stop();
}


void TaskScheduler::Run() {
    io_context_.run();
}


void TaskScheduler::Stop() {
    work_guard_.reset();
    io_context_.stop();
    if (thread_pool_) {
        thread_pool_->join();
    }
}


void TaskScheduler::ScheduleTask(std::shared_ptr<AsyncTask> task
                                , AsyncTask::CompletionHandler handler)
{
    post(io_context_, [this, task, handler]()
        {
            ProcessTask(task, handler);
        }
    );
}


void TaskScheduler::ScheduleDelayedTask(std::shared_ptr<AsyncTask> task
                                        , AsyncTask::CompletionHandler handler
                                        , std::chrono::milliseconds delay)
{
    auto timer = std::make_shared<boost::asio::steady_timer>(io_context_, delay);

    timer->async_wait([this, task, handler, timer](const boost::system::error_code& ec)
        {
            if (!ec) {
                ProcessTask(task, handler);
            }
        }
    );
}


boost::asio::io_context& TaskScheduler::GetIOContext() {
    return io_context_;
}


bool TaskScheduler::IsTaskRunning(const std::string& task_id) const {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    auto it = running_tasks_.find(task_id);
    return (it != running_tasks_.end() && it->second);
}


void TaskScheduler::ProcessTask(std::shared_ptr<AsyncTask> task
                                , AsyncTask::CompletionHandler handler)
{
    {
        std::lock_guard<std::mutex> lock(tasks_mutex_);
        running_tasks_[task->GetTaskId()] = true;
    }

    post(*thread_pool_, [this, task, handler]()
        {
            task->Execute([this, task, handler](bool success, const std::string& result)
                {
                    post(io_context_, [this, task, handler, success, result]()
                        {
                            {
                                std::lock_guard<std::mutex> lock(tasks_mutex_);
                                running_tasks_[task->GetTaskId()] = false;
                            }
                            handler(success, result);
                        });
                });
        });
}


ApiRequestTask::ApiRequestTask(const std::string& task_id
                            , const std::string& url
                            , const std::string& api_key)
    : AsyncTask(task_id)
    , url_(url)
    , api_key_(api_key)
{}


void ApiRequestTask::Execute(CompletionHandler handler) {
    try {
        cpr::Response response = cpr::Get(cpr::Url{url_});

        if (response.status_code == 200) {
            try {
                nlohmann::json json_response =
                    nlohmann::json::parse(response.text);

                handler(true, response.text);
            } catch (const nlohmann::json::parse_error& e) {
                handler(false, "JSON parse error: "
                        + std::string(e.what()));
            }

        } else {
            handler(false, "HTTP error: "
                + std::to_string(response.status_code));
        }

    } catch (const std::exception& e) {
        handler(false, "Request failed: "
            + std::string(e.what()));
    }
}


CacheReadTask::CacheReadTask(const std::string& task_id
                        , const std::string& cache_path)
    : AsyncTask(task_id)
    , cache_path_(cache_path)
{}


void CacheReadTask::Execute(CompletionHandler handler) {
    try {
        std::ifstream file(cache_path_);
        if (!file.is_open()) {
            handler(false, "Cache file not found");
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file))
                            , std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            handler(false, "Cache file is empty");
        } else {
            handler(true, content);
        }

    } catch (const std::exception& e) {
        handler(false, "Cache read error: "
            + std::string(e.what()));
    }
}


CacheWriteTask::CacheWriteTask(const std::string& task_id
                            , const std::string& cache_path
                            , const std::string& data)
    : AsyncTask(task_id)
    , cache_path_(cache_path)
    , data_(data)
{}


void CacheWriteTask::Execute(CompletionHandler handler) {
    try {
        std::ofstream file(cache_path_);
        if (!file.is_open()) {
            handler(false, "Cannot create cache file");
            return;
        }

        file << data_;
        file.close();
        handler(true, "Cache written successfully");

    } catch (const std::exception& e) {
        handler(false, "Cache write error: "
            + std::string(e.what()));
    }
}
