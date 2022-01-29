
#include <memory>
#include <string>

#include "rtc/modules/video_capture/video_capture_factory.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/thread.h"

class CaptureFrameSink : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  void OnFrame(const webrtc::VideoFrame& frame) override {
    RTC_LOG(LS_INFO) << "capture frame:" << frame.width() << "x"
                     << frame.height();
  }

  void OnDiscardedFrame() override {
    RTC_LOG(LS_WARNING) << ">>>>> discard frame";
  }
};

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto capture_device_info = webrtc::VideoCaptureFactory::CreateDeviceInfo();

  auto device_count = capture_device_info->NumberOfDevices();
  if (device_count < 1) {
    RTC_LOG(LS_ERROR) << "cannot find video capture!!!";
    return -1;
  }
  RTC_LOG(LS_INFO) << "find video capture device count:" << device_count;

  char device_name[256] = {0};
  char unique_name[256] = {0};
  if (capture_device_info->GetDeviceName(0, device_name, sizeof(device_name),
                                         unique_name,
                                         sizeof(unique_name)) != 0) {
    RTC_LOG(LS_ERROR) << "get video capture device name failed";
    return -1;
  }
  RTC_LOG(LS_INFO) << "/***************************************************/";
  RTC_LOG(LS_INFO) << "video capture device name:" << device_name;
  RTC_LOG(LS_INFO) << "video capture unique name:" << unique_name;

  webrtc::VideoCaptureCapability request_capability;
  request_capability.width = 1920;
  request_capability.height = 1080;
  request_capability.maxFPS = 25;
  webrtc::VideoCaptureCapability capability;
  if (capture_device_info->GetBestMatchedCapability(
          unique_name, request_capability, capability) < 1) {
    RTC_LOG(LS_ERROR) << "get video capture device capability failed";
    return -1;
  }
  RTC_LOG(LS_INFO) << "/***************************************************/";
  RTC_LOG(LS_INFO) << "width:" << capability.width;
  RTC_LOG(LS_INFO) << "height:" << capability.height;
  RTC_LOG(LS_INFO) << "maxFPS:" << capability.maxFPS;
  RTC_LOG(LS_INFO) << "videoType:" << capability.videoType;

  auto vcm = webrtc::VideoCaptureFactory::Create(unique_name);
  auto frame_sink = std::make_unique<CaptureFrameSink>();
  vcm->RegisterCaptureDataCallback(frame_sink.get());

  RTC_LOG(LS_INFO) << ">>>>> start capture";
  vcm->StartCapture(capability);
  rtc::Thread::SleepMs(5000);
  vcm->StopCapture();
  RTC_LOG(LS_INFO) << ">>>>> stop capture";

  return 0;
}
