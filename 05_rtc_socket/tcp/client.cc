
#include <cstdlib>
#include <memory>

#include "rtc/rtc_base/async_tcp_socket.h"
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/physical_socket_server.h"
#include "rtc/rtc_base/task_utils/to_queued_task.h"
#include "rtc/rtc_base/thread.h"

class Client : public sigslot::has_slots<> {
 public:
  explicit Client(rtc::AsyncPacketSocket* socket) : socket_(socket) {
    socket_->SignalConnect.connect(this, &Client::OnConnect);
    socket_->SignalClose.connect(this, &Client::OnClose);
    socket_->SignalReadPacket.connect(this, &Client::OnMsg);
  }

  void SendMsg() {
    rtc::ThreadManager::Instance()->CurrentThread()->PostDelayedTask(
        webrtc::ToQueuedTask([this]() {
          std::string msg = "hello " + std::to_string(++count_);
          socket_->Send(msg.c_str(), msg.size(), rtc::PacketOptions{});
        }),
        500);
  }

  void OnMsg(rtc::AsyncPacketSocket* socket, const char* data, size_t len,
             const rtc::SocketAddress& remote_addr,
             const int64_t& packet_time_us) {
    std::string message(data, len);
    RTC_LOG(LS_INFO) << "receive msg: " << message << " from "
                     << remote_addr.ToString();
    SendMsg();
  }

  void OnClose(rtc::AsyncPacketSocket*, int code) {
    RTC_LOG(LS_INFO) << "==== close:" << code;
    delete this;
  }

  void OnConnect(rtc::AsyncPacketSocket*) {
    RTC_LOG(LS_INFO) << ">>>>> connect success";
    RTC_LOG(LS_INFO) << "local_addr:" << socket_->GetLocalAddress().ToString();
    RTC_LOG(LS_INFO) << "remote_addr:"
                     << socket_->GetRemoteAddress().ToString();
    SendMsg();
  }

 private:
  std::unique_ptr<rtc::AsyncPacketSocket> socket_;
  int64_t count_{0};
};

int main(int argc, char* argv[]) {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_INFO);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  uint16_t port = 8888;
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }
  rtc::SocketAddress local_addr("0.0.0.0", 0);
  rtc::SocketAddress server_addr("0.0.0.0", port);
  auto io_thread = rtc::Thread::CreateWithSocketServer();

  auto socket = io_thread->socketserver()->CreateSocket(server_addr.family(),
                                                        SOCK_STREAM);

  auto client_socket =
      rtc::AsyncTCPSocket::Create(socket, local_addr, server_addr);
  if (!client_socket) {
    RTC_LOG(LS_ERROR) << ">>>>> bind or connect failed";
    return -1;
  }

  auto client = std::make_unique<Client>(client_socket);

  io_thread->Start();

  RTC_LOG(LS_INFO) << "hello rtc";
  rtc::Event wait;
  wait.Wait(rtc::Event::kForever);
  return 0;
}
