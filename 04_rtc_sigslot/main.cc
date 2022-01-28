
#include <string>

#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/task_utils/to_queued_task.h"
#include "rtc/rtc_base/third_party/sigslot/sigslot.h"
#include "rtc/rtc_base/thread.h"

class MySlot : public sigslot::has_slots<sigslot::multi_threaded_local> {
 public:
  void OnMsg(std::string msg) {
    RTC_LOG(LS_INFO) << ">>>>> receive msg:" << msg;
  }
};

class MySignal {
 public:
  sigslot::signal1<std::string, sigslot::multi_threaded_local> signal;
};

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto thread = rtc::Thread::Create();
  thread->Start();

  MySignal mySignal;

  MySlot mySlot1;
  MySlot* mySlot2 = new MySlot();

  rtc::Event wait;
  thread->PostTask(webrtc::ToQueuedTask([&mySignal, &mySlot1, &wait]() {
    mySignal.signal.connect(&mySlot1, &MySlot::OnMsg);
    wait.Set();
  }));
  mySignal.signal.connect(mySlot2, &MySlot::OnMsg);

  wait.Wait(rtc::Event::kForever);
  delete mySlot2;

  mySignal.signal.emit("hello sigslot");

  return 0;
}
