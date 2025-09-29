#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>
#include <string>
#include <future>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>


class AsyncTask {
public:
    using CompletionHandler = std::function<void(bool success, const std::string& result)>;

    explicit AsyncTask(const std::string& task_id) : task_id_(task_id) {}

    virtual ~AsyncTask() = default;

    virtual void Execute(CompletionHandler handler) = 0;

    const std::string& GetTaskId() const { return task_id_; }

protected:
    std::string task_id_;
};


class TaskScheduler {
public:
    TaskScheduler(size_t thread_pool_size = 4);

    ~TaskScheduler();

public:
    void Run();

    void Stop();

    void ScheduleTask(std::shared_ptr<AsyncTask> task
                    , AsyncTask::CompletionHandler handler);

    void ScheduleDelayedTask(std::shared_ptr<AsyncTask> task
                        , AsyncTask::CompletionHandler handler
                        , std::chrono::milliseconds delay);

    boost::asio::io_context& GetIOContext();

    bool IsTaskRunning(const std::string& task_id) const;

private:
    void ProcessTask(std::shared_ptr<AsyncTask> task
                    , AsyncTask::CompletionHandler handler);

private:
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::io_context::work> work_guard_;
    std::unique_ptr<boost::asio::thread_pool> thread_pool_;
    std::unordered_map<std::string, bool> running_tasks_;
    mutable std::mutex tasks_mutex_;
};


class ApiRequestTask : public AsyncTask {
public:
    ApiRequestTask(const std::string& task_id
                , const std::string& url
                , const std::string& api_key);

    void Execute(CompletionHandler handler) override;

private:
    std::string url_;
    std::string api_key_;
};


class CacheReadTask : public AsyncTask {
public:
    CacheReadTask(const std::string& task_id
            , const std::string& cache_path);

    void Execute(CompletionHandler handler) override;

private:
    std::string cache_path_;
};


class CacheWriteTask : public AsyncTask {
public:
    CacheWriteTask(const std::string& task_id
                , const std::string& cache_path
                , const std::string& data);

    void Execute(CompletionHandler handler) override;

private:
    std::string cache_path_;
    std::string data_;
};
