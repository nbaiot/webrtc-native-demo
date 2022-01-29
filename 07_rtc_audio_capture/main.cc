
#include "rtc/modules/audio_device/include/audio_device.h"
#include "rtc/modules/audio_device/include/audio_device_data_observer.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/task_queue_libevent.h"
#include "rtc/rtc_base/thread.h"

class AudioCaptureObserver : public webrtc::AudioDeviceDataObserver {
 public:
  ~AudioCaptureObserver() override = default;

  void OnCaptureData(const void* audio_samples, const size_t num_samples,
                     const size_t bytes_per_sample, const size_t num_channels,
                     const uint32_t samples_per_sec) override {
    RTC_LOG(LS_INFO) << "OnCaptureData "
                     << "bytes_per_sample:" << bytes_per_sample
                     << ",num_channels:" << num_channels
                     << ",samples_per_sec:" << samples_per_sec
                     << ", num_samples:" << num_samples;
  }

  void OnRenderData(const void* audio_samples, const size_t num_samples,
                    const size_t bytes_per_sample, const size_t num_channels,
                    const uint32_t samples_per_sec) override {
    RTC_LOG(LS_INFO) << "OnRenderData "
                     << "bytes_per_sample:" << bytes_per_sample
                     << ",num_channels:" << num_channels
                     << ",samples_per_sec:" << samples_per_sec
                     << ", num_samples:" << num_samples;
  }
};

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto task_queue_factory = webrtc::CreateTaskQueueLibeventFactory();
  auto adm = webrtc::CreateAudioDeviceWithDataObserver(
      webrtc::AudioDeviceModule::AudioLayer::kLinuxPulseAudio,
      task_queue_factory.get(),
      std::make_unique<AudioCaptureObserver>()
      );

  if (adm->Init() != 0) {
    RTC_LOG(LS_ERROR) << "aduio device module init failed";
    return -1;
  }

  RTC_LOG(LS_INFO) << ">>>>>>>>>>>>>>>>>>>>> audio capture device count:"
                   << adm->RecordingDevices();

  char name[webrtc::kAdmMaxDeviceNameSize] = {0};
  char guid[webrtc::kAdmMaxGuidSize] = {0};
  for (auto i = 0; i < adm->RecordingDevices(); ++i) {
    std::memset(name, 0, webrtc::kAdmMaxDeviceNameSize);
    std::memset(guid, 0, webrtc::kAdmMaxGuidSize);
    if (adm->RecordingDeviceName(i, name, guid) == 0) {
      RTC_LOG(LS_INFO) << ">>>>> audio capture device name:" << name;
    }
  }

  adm->SetRecordingDevice(0);
  if (adm->InitRecording() != 0) {
    RTC_LOG(LS_INFO) << ">>>>>>>>>> audio capture device init failed";
    return -1;
  }

  RTC_LOG(LS_INFO) << ">>>>> start recording";
  adm->StartRecording();
  rtc::Thread::SleepMs(5000);
  adm->StopRecording();
  RTC_LOG(LS_INFO) << ">>>>> stop recording";

  return 0;
}
