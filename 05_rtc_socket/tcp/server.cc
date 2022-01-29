
#include <memory>

#include "rtc/rtc_base/async_tcp_socket.h"
#include "rtc/rtc_base/event.h"
#include "rtc/rtc_base/logging.h"
#include "rtc/rtc_base/physical_socket_server.h"
#include "rtc/rtc_base/thread.h"

class Echo : public sigslot::has_slots<> {
 public:
  explicit Echo(rtc::AsyncPacketSocket* socket) : socket_(socket) {
    socket->SignalReadPacket.connect(this, &Echo::OnMsg);
    socket_->SignalClose.connect(this, &Echo::OnClose);
  }

  void OnMsg(rtc::AsyncPacketSocket* socket, const char* data, size_t len,
             const rtc::SocketAddress& remote_addr,
             const int64_t& packet_time_us) {
    std::string message(data, len);
    RTC_LOG(LS_INFO) << "receive msg: " << message << " from "
                     << remote_addr.ToString();
    socket_->Send(data, len, rtc::PacketOptions{});
  }

  void OnClose(rtc::AsyncPacketSocket*, int code) {
    RTC_LOG(LS_INFO) << "==== close:" << code;
    delete this;
  }

 private:
  std::unique_ptr<rtc::AsyncPacketSocket> socket_;
};

class NewTcpConnection : public sigslot::has_slots<> {
 public:
  void OnNewConnection(rtc::AsyncListenSocket* socket,
                       rtc::AsyncPacketSocket* new_async_tcp_socket) {
    RTC_LOG(LS_INFO) << ">>>>> new clinet "
                     << new_async_tcp_socket->GetRemoteAddress().ToString();
    new Echo(new_async_tcp_socket);
  }
};

int main(int argc, char* argv[]) {
  // init log
  rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
  rtc::LogMessage::LogThreads(true);
  rtc::LogMessage::LogTimestamps(true);

  uint16_t port = 8888;
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }
  auto listen_thread = rtc::Thread::CreateWithSocketServer();
  // SocketDispatcher
  rtc::SocketAddress server_addr("0.0.0.0", port);
  auto socket = listen_thread->socketserver()->CreateSocket(
      server_addr.family(), SOCK_STREAM);

  if (socket->Bind(server_addr) != 0) {
    RTC_LOG(LS_ERROR) << "bind " << server_addr.ToString() << " failed";
    return -1;
  }

  listen_thread->Start();

  // listen(max listen: 5) and wait accept
  auto tcp_listen_socket = std::make_unique<rtc::AsyncTcpListenSocket>(
      std::unique_ptr<rtc::Socket>(socket));

  NewTcpConnection tcp_connection;
  tcp_listen_socket->SignalNewConnection.connect(
      &tcp_connection, &NewTcpConnection::OnNewConnection);

  RTC_LOG(LS_INFO) << ">>>>> wait client ...";

  rtc::Event wait;
  wait.Wait(rtc::Event::kForever);
  return 0;
}
