#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/platform_thread.h"
#include "rtc/rtc_base/thread.h"

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  bool exit_thread = false;
  auto thread = rtc::PlatformThread::SpawnJoinable(
      [&exit_thread]() {
        RTC_LOG(LS_INFO) << "rtc platform thread started";
        while (!exit_thread) {
          RTC_LOG(LS_INFO) << "I am from rtc platform thread";
          rtc::Thread::SleepMs(500);
        }
        RTC_LOG(LS_INFO) << "rtc platform thread exited";
      },
      "test_rtc_thread");

  rtc::Thread::SleepMs(2600);
  exit_thread = true;
  thread.Finalize();

  return 0;
}
