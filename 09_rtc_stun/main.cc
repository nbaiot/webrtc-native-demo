
#include <memory>
#include <vector>

#include "p2p/base/basic_packet_socket_factory.h"
#include "p2p/stunprober/stun_prober.h"
#include "rtc/p2p/base/stun_request.h"
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/helpers.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/network.h"
#include "rtc/rtc_base/ssl_adapter.h"
#include "rtc/rtc_base/thread.h"
#include "rtc/rtc_base/time_utils.h"

class StunBindingRequest : public cricket::StunRequest {
 public:
  void OnResponse(cricket::StunMessage* response) override {}

  void OnErrorResponse(cricket::StunMessage* response) override {}

  void OnTimeout() override{};
};

class NetworkManagerSlot : public sigslot::has_slots<> {
 public:
  explicit NetworkManagerSlot(rtc::Event* update) : updated_(update) {}

  void OnNetworksChanged() {
    RTC_LOG(LS_INFO) << ">>>>> network updated";
    updated_->Set();
  }

 private:
  rtc::Event* updated_{nullptr};
};

const char* PrintNatType(stunprober::NatType type) {
  switch (type) {
    case stunprober::NATTYPE_NONE:
      return "Not behind a NAT";
    case stunprober::NATTYPE_UNKNOWN:
      return "Unknown NAT type";
    case stunprober::NATTYPE_SYMMETRIC:
      return "Symmetric NAT";
    case stunprober::NATTYPE_NON_SYMMETRIC:
      return "Non-Symmetric NAT";
    default:
      return "Invalid";
  }
}

void PrintStats(stunprober::StunProber* prober) {
  stunprober::StunProber::Stats stats;
  if (!prober->GetStats(&stats)) {
    RTC_LOG(LS_WARNING) << "Results are inconclusive.";
    return;
  }

  RTC_LOG(LS_INFO) << "Shared Socket Mode: " << stats.shared_socket_mode;
  RTC_LOG(LS_INFO) << "Requests sent: " << stats.num_request_sent;
  RTC_LOG(LS_INFO) << "Responses received: " << stats.num_response_received;
  RTC_LOG(LS_INFO) << "Target interval (ns): "
                   << stats.target_request_interval_ns;
  RTC_LOG(LS_INFO) << "Actual interval (ns): "
                   << stats.actual_request_interval_ns;
  RTC_LOG(LS_INFO) << "NAT Type: " << PrintNatType(stats.nat_type);
  RTC_LOG(LS_INFO) << "Host IP: " << stats.host_ip;
  RTC_LOG(LS_INFO) << "Server-reflexive ips: ";
  for (auto& ip : stats.srflx_addrs) {
    RTC_LOG(LS_INFO) << "\t" << ip;
  }

  RTC_LOG(LS_INFO) << "Success Precent: " << stats.success_percent;
  RTC_LOG(LS_INFO) << "Response Latency:" << stats.average_rtt_ms;
}

void StopTrial(stunprober::StunProber* prober, int result) {
  if (prober) {
    RTC_LOG(LS_INFO) << "Result: " << result;
    if (result == stunprober::StunProber::SUCCESS) {
      PrintStats(prober);
    }
  }
}

int main(int argc, char* argv[]) {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  if (argc < 2 || argc > 3) {
    RTC_LOG(LS_ERROR) << "usage: stunserver address";
    return -1;
  }
  std::vector<rtc::SocketAddress> server_addresses;
  rtc::SocketAddress server_addr;
  if (!server_addr.FromString(argv[1])) {
    RTC_LOG(LS_ERROR) << "Unable to parse IP address: " << argv[1];
    return -1;
  }
  server_addresses.push_back(server_addr);
  if (argc == 3) {
    rtc::SocketAddress server_addr2;
    if (!server_addr2.FromString(argv[2])) {
      RTC_LOG(LS_ERROR) << "Unable to parse IP address: " << argv[1];
      return -1;
    }
    server_addresses.push_back(server_addr2);
  }

  if (!rtc::InitializeSSL()) {
    RTC_LOG(LS_ERROR) << "init ssl failed";
    return -1;
  }
  RTC_LOG(LS_INFO) << "init ssl success";

  if (!rtc::InitRandom(rtc::Time32())) {
    RTC_LOG(LS_ERROR) << "init random failed";
    return -1;
  }
  RTC_LOG(LS_INFO) << "init random success";

  auto net_thread = rtc::Thread::CreateWithSocketServer();
  net_thread->Start();
  auto network_manager =
      std::make_unique<rtc::BasicNetworkManager>(net_thread->socketserver());
  network_manager->Initialize();
  rtc::Event wait_update;
  auto network_manager_slots =
      std::make_unique<NetworkManagerSlot>(&wait_update);

  network_manager->SignalNetworksChanged.connect(
      network_manager_slots.get(), &NetworkManagerSlot::OnNetworksChanged);

  net_thread->Invoke<void>(RTC_FROM_HERE, [&network_manager]() {
    RTC_LOG(LS_INFO) << "start updating...";
    network_manager->StartUpdating();
  });

  RTC_LOG(LS_INFO) << "wait network update ...";
  wait_update.Wait(rtc::Event::kForever);
  rtc::NetworkManager::NetworkList networks;
  network_manager->GetNetworks(&networks);

  RTC_LOG(LS_INFO) << "network count:" << networks.size();

  auto packet_socket_factory = std::make_unique<rtc::BasicPacketSocketFactory>(
      net_thread->socketserver());
  auto stun_prober = std::make_unique<stunprober::StunProber>(
      packet_socket_factory.get(), net_thread.get(), networks);

  stun_prober->Start(server_addresses, true, 10, 2, 1000,
                     [](stunprober::StunProber* prober, int result) {
                       StopTrial(prober, result);
                     });

  rtc::Event wait;
  wait.Wait(rtc::Event::kForever);
  return 0;
}
