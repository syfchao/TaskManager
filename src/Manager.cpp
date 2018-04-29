#include "Manager.hh"

namespace Task {

Manager::Manager(std::shared_ptr<Detail::Threadpool> Threadpool, size_t maxWorkers)
  : _threadpool(std::move(Threadpool))
  , _maxWorkers(maxWorkers) {}

std::future<void> Manager::stop() {
  auto task = std::make_shared<std::packaged_task<void()>>([] {});
  auto future = task->get_future();

  auto functor = [task = std::move(task)]() mutable {
    (*task)();
  };
  this->addTask(std::move(functor));

  return future;
}

void Manager::addTask(std::function<void()> functor) {
  std::lock_guard<std::mutex> guard(_mutex);

  _tasks.emplace(std::move(functor), Detail::Clock::now());
  this->processTasks();
}

void Manager::processTasks() {
  if (_tasks.empty() || _workerCount == _maxWorkers) {
    return;
  }
  auto task = std::move(_tasks.front());
  _tasks.pop();

  ++_workerCount;
  _threadpool->schedule(std::move(task));
}

} /* namespace Task */
