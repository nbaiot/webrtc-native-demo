
#include "rtc/rtc_base/logging.h"

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  RTC_LOG(LS_INFO) << "hello rtc";
  return 0;
}
