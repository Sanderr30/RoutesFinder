
### Отложеннная обработка (TTaskScheduler)


```cpp
class TaskScheduler {
    void ScheduleTask(std::shared_ptr<AsyncTask> task, CompletionHandler handler);
    void ScheduleDelayedTask(std::shared_ptr<AsyncTask> task,
                           CompletionHandler handler,
                           std::chrono::milliseconds delay);
};
```

Использует `boost::asio::io_context` основной event loop
Задачи планируются через `post()` и выполняются асинхронно
Поддерживает отложенное выполнение через `boost::asio::steady_timer`
Отслеживает состояние выполняющихся задач

### Обход синхронности CPR

Использование `boost::asio::thread_pool`

```cpp
void TaskScheduler::ProcessTask(std::shared_ptr<AsyncTask> task,
                               AsyncTask::CompletionHandler handler) {
    post(*thread_pool_, [this, task, handler]() {
        task->Execute([this, task, handler](bool success, const std::string& result) {
            post(io_context_, [this, task, handler, success, result]() {
                handler(success, result);
            });
        });
    });
}
```

### Асинхронные системные вызовы


1. **Файловые операции:** `CacheReadTask`, `CacheWriteTask`
2. **HTTP запросы:** `ApiRequestTask`
3. **Ввод пользователя:** `AsyncInputHandler`

```cpp
class CacheReadTask : public AsyncTask {
    void Execute(CompletionHandler handler) override {
        std::ifstream file(cache_path_);
        handler(success, content);
    }
};
```

### Неблокирующий ввод в режиме "запрос-ответ"

`AsyncInputHandler`

```cpp
void AsyncInputHandler::ReadInputLoop() {
    while (!should_stop_.load()) {
        std::string input;
        if (std::getline(std::cin, input)) {
            {
                std::lock_guard<std::mutex> lock(input_mutex_);
                input_queue_.push(input);
            }
            post(io_context_, [this]() {
                ProcessInput();
            });
        }
    }
}
```

### Idling Task для поддержания Event Loop

Фиктивная задача, которая постоянно перепланирует себя:

```cpp
void AsyncInputHandler::ScheduleIdleTask() {
    idle_timer_->expires_after(std::chrono::milliseconds(100));
    idle_timer_->async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !should_stop_.load()) {
            ScheduleIdleTask();
        }
    });
}
```







## Детали реализации

### TaskScheduler

```cpp
class TaskScheduler {
private:
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::io_context::work> work_guard_;
    std::unique_ptr<boost::asio::thread_pool> thread_pool_;
    std::unordered_map<std::string, bool> running_tasks_;
};
```


- Планирование задач: `ScheduleTask()`
- Отложенное выполнение: `ScheduleDelayedTask()`
- Предотвращение дублирования: `IsTaskRunning()`
- Graceful shutdown: `Stop()`


### AsyncTask - Базовый класс для всех задач

```cpp
class AsyncTask {
public:
    using CompletionHandler = std::function<void(bool success, const std::string& result)>;
    virtual void Execute(CompletionHandler handler) = 0;
};
```


- **ApiRequestTask** - HTTP запросы к API
- **CacheReadTask** - Чтение кеша
- **CacheWriteTask** - Запись кеша

### AsyncInputHandler - Неблокирующий ввод

- Отдельный поток для чтения
- Thread-safe очередь для передачи данных
- Idling task для поддержания event loop
- Thread-safe вывод сообщений



### AsyncApiManager - Менеджер API запросов

```cpp
class AsyncApiManager {
public:
    void GetRoutesAsync(const ConfigVariables& config, ApiResponseCallback callback);
private:
    TaskScheduler& scheduler_;
    std::string api_key_;
};
```


- Построение URL для API
- Предотвращение дублирующих запросов
- Парсинг JSON ответов
- Обработка ошибок
