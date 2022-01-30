
#include <memory>

#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/network.h"
#include "rtc/rtc_base/thread.h"

class NetworkManagerSlot : public sigslot::has_slots<> {
 public:
  explicit NetworkManagerSlot(rtc::Event* update) : updated_(update) {}
  void OnError() { RTC_LOG(LS_ERROR) << ">>>>> network_manager error"; }

  void OnNetworksChanged() {
    RTC_LOG(LS_INFO) << ">>>>> network updated";
    updated_->Set();
  }

 private:
  rtc::Event* updated_{nullptr};
};

int main() {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  auto net_thread = rtc::Thread::CreateWithSocketServer();
  net_thread->Start();

  auto network_manager =
      std::make_unique<rtc::BasicNetworkManager>(net_thread->socketserver());
  network_manager->Initialize();
  RTC_LOG(LS_INFO) << "enumeration permission:"
                   << (network_manager->enumeration_permission() ==
                               rtc::NetworkManagerBase::ENUMERATION_ALLOWED
                           ? "allowed"
                           : "disabled");

  rtc::Event wait_update;
  auto network_manager_slots =
      std::make_unique<NetworkManagerSlot>(&wait_update);
  network_manager->SignalError.connect(network_manager_slots.get(),
                                       &NetworkManagerSlot::OnError);

  network_manager->SignalNetworksChanged.connect(
      network_manager_slots.get(), &NetworkManagerSlot::OnNetworksChanged);

  net_thread->Invoke<void>(RTC_FROM_HERE, [&network_manager]() {
    RTC_LOG(LS_INFO) << "start updating...";
    network_manager->StartUpdating();
  });

  RTC_LOG(LS_INFO) << "wait network update ...";
  wait_update.Wait(rtc::Event::kForever);

  rtc::IPAddress default_ip_address;
  rtc::NetworkManager::NetworkList network_list;
  network_manager->GetNetworks(&network_list);
  if (network_list.empty()) {
    network_manager->GetAnyAddressNetworks(&network_list);
  }
  if (network_list.empty()) {
    RTC_LOG(LS_ERROR) << "get network list failed";
  }
  for (const auto& network : network_list) {
    RTC_LOG(LS_INFO) << "network:" << network->ToString();
    RTC_LOG(LS_INFO) << ">>>>> ip:" << network->GetBestIP().ToString();
    RTC_LOG(LS_INFO) << ">>>>> cost:" << network->GetCost();
  }
  if (network_manager->GetDefaultLocalAddress(AF_INET, &default_ip_address)) {
    RTC_LOG(LS_INFO) << "default_ip_address:" << default_ip_address.ToString();
  }

  net_thread->Invoke<void>(RTC_FROM_HERE, [&network_manager]() {
    RTC_LOG(LS_INFO) << "stop updating...";
    network_manager->StopUpdating();
  });

  net_thread->Stop();
  return 0;
}
