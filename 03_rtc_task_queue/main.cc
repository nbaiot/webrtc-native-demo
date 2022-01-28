
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/task_queue.h"
#include "rtc/rtc_base/task_queue_libevent.h"

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto task_queue_factory = webrtc::CreateTaskQueueLibeventFactory();
  auto task_queue_base = task_queue_factory->CreateTaskQueue(
      "std_task_queue", webrtc::TaskQueueFactory::Priority::NORMAL);

  auto task_queue =
      std::make_unique<rtc::TaskQueue>(std::move(task_queue_base));

  task_queue->PostTask(
      []() { RTC_LOG(LS_INFO) << "I am from rtc task queue"; });

  task_queue->PostDelayedTask(
      []() { RTC_LOG(LS_INFO) << "I am from rtc task queue, delayed 500ms"; },
      500);

  rtc::Event wait;
  wait.Wait(600);
  return 0;
}
