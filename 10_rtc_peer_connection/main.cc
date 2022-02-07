
#include "rtc/api/audio_codecs/builtin_audio_decoder_factory.h"
#include "rtc/api/audio_codecs/builtin_audio_encoder_factory.h"
#include "rtc/api/create_peerconnection_factory.h"
#include "rtc/api/jsep.h"
#include "rtc/api/video_codecs/builtin_video_decoder_factory.h"
#include "rtc/api/video_codecs/builtin_video_encoder_factory.h"
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/thread.h"

class PCObserver : public webrtc::PeerConnectionObserver {
 public:
  ~PCObserver() override = default;
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> signaling_state:"
                        << webrtc::PeerConnectionInterface::AsString(new_state);
  }

  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> data channel id:" << data_channel->id();
  }

  void OnRenegotiationNeeded() override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> OnRenegotiationNeeded";
  }

  void OnStandardizedIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> IceConnectionState:"
                        << webrtc::PeerConnectionInterface::AsString(new_state);
  }

  void OnConnectionChange(
      webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> PeerConnectionState:"
                        << webrtc::PeerConnectionInterface::AsString(new_state);
  }

  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> IceGatheringState:"
                        << webrtc::PeerConnectionInterface::AsString(new_state);
  }

  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
    std::string ice_candidate;
    if (!candidate->ToString(&ice_candidate)) {
      RTC_LOG(LS_WARNING) << ">>>>>>>>>> candidate to string failed";
    }
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> IceCandidate:" << ice_candidate;
  }

  void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
      override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> OnTrack id:"
                        << transceiver->receiver()->id();
  }

  void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> OnRemoveTrack id:" << receiver->id();
  }
};

class SetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  ~SetSessionDescriptionObserver() override = default;
  void OnSuccess() override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> set description success";
  }
  void OnFailure(webrtc::RTCError error) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> set description failed:"
                        << error.message();
  }

  static SetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<SetSessionDescriptionObserver>();
  }
};

class SessionDescriptionObserver
    : public webrtc::CreateSessionDescriptionObserver {
 public:
  explicit SessionDescriptionObserver(webrtc::PeerConnectionInterface* pc)
      : pc_(pc) {}
  ~SessionDescriptionObserver() override = default;

  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> type:" << desc->type();
    std::string sdp;
    desc->ToString(&sdp);
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> desc:" << sdp;
    if (desc->GetType() == webrtc::SdpType::kOffer) {
      pc_->SetLocalDescription(SetSessionDescriptionObserver::Create(), desc);
    }
  }
  void OnFailure(webrtc::RTCError error) override {
    RTC_LOG(LS_WARNING) << ">>>>>>>>>> error:" << error.message();
  }

 private:
  webrtc::PeerConnectionInterface* pc_{nullptr};
};

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_WARNING);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto signaling_thread = rtc::Thread::CreateWithSocketServer();
  signaling_thread->Start();

  auto peer_connection_factory = webrtc::CreatePeerConnectionFactory(
      nullptr /* network_thread */, nullptr /* worker_thread */,
      signaling_thread.get(), nullptr /* default_adm */,
      webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      webrtc::CreateBuiltinVideoEncoderFactory(),
      webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
      nullptr /* audio_processing */
  );

  if (!peer_connection_factory) {
    RTC_LOG(LS_WARNING) << "create peer_connection_factory failed";
    return -1;
  }

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  webrtc::PeerConnectionInterface::IceServer server;
  server.uri = "stun:stun.l.google.com:19302";
  config.servers.push_back(server);

  auto pc_observer = std::make_unique<PCObserver>();
  webrtc::PeerConnectionDependencies dependencies(pc_observer.get());
  auto ret = peer_connection_factory->CreatePeerConnectionOrError(
      config, std::move(dependencies));
  if (!ret.ok()) {
    RTC_LOG(LS_WARNING) << "create peer connection failed";
    return -1;
  }
  auto pc = ret.MoveValue();

  auto audio_track = peer_connection_factory->CreateAudioTrack(
      "audio_label",
      peer_connection_factory->CreateAudioSource(cricket::AudioOptions()));
  auto result_or_error = pc->AddTrack(audio_track, {"stream_id"});
  if (!result_or_error.ok()) {
    RTC_LOG(LS_WARNING) << "failed to add audio track to PeerConnection:"
                        << result_or_error.error().message();
    return -1;
  }

  rtc::scoped_refptr<SessionDescriptionObserver> session_observer(
      new rtc::RefCountedObject<SessionDescriptionObserver>(pc.get()));
  pc->CreateOffer(session_observer.get(),
                  webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

  rtc::Event wait;
  wait.Wait(rtc::Event::kForever);
  RTC_LOG(LS_WARNING) << "hello rtc";
  return 0;
}
