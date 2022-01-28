#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/platform_thread.h"
#include "rtc/rtc_base/task_utils/to_queued_task.h"
#include "rtc/rtc_base/thread.h"

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  RTC_LOG(LS_INFO) << "is main thread:"
                   << (rtc::ThreadManager::Instance()->IsMainThread()
                           ? "true"
                           : "false");

  auto rtc_thread = rtc::Thread::Create();
  rtc_thread->SetName("rtc_thread", rtc_thread.get());
  rtc_thread->Start();

  rtc_thread->PostTask(webrtc::ToQueuedTask(
      []() { RTC_LOG(LS_INFO) << "I am from rtc thread by PostTask1"; }));

  rtc_thread->PostTask(RTC_FROM_HERE, []() {
      RTC_LOG(LS_INFO) << "I am from rtc thread by PostTask2";
  });

  rtc_thread->PostDelayedTask(webrtc::ToQueuedTask([]() {
                                RTC_LOG(LS_INFO)
                                    << "I am from rtc thread, delay 1000ms";
                              }),
                              1000);


  RTC_LOG(LS_INFO) << ">>>>> before Invoke";
  rtc_thread->Invoke<void>(RTC_FROM_HERE, [](){
      rtc::Thread::SleepMs(500);
      RTC_LOG(LS_INFO) << "I am from rtc thread by Invoke";
  });
  RTC_LOG(LS_INFO) << ">>>>> after Invoke";

  rtc::Thread::SleepMs(1000);
  rtc_thread->Stop();

  return 0;
}
