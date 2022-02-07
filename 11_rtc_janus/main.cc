
#include "janus_session.h"
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/thread.h"

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  RTC_LOG(LS_INFO) << "hello rtc";
  auto janus_session = std::make_unique<JanusSession>("106.53.67.18", 8188);

  janus_session->Init();

  rtc::Thread::SleepMs(12000);

  janus_session->Destory();

  rtc::Event wait;
  wait.Wait(rtc::Event::kForever);
  RTC_LOG(LS_INFO) << "hello rtc";
  return 0;
}
