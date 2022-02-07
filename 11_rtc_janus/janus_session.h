#pragma once

#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <functional>
#include <map>
#include <mutex>
#include <thread>

#include "nlohmann/json.hpp"
#include "websocket_transport.h"

class JanusSession {
 public:
  using OnReadyCallback = std::function<void()>;
  using OnResponseCallback =
      std::function<void(const nlohmann::json& response)>;
  JanusSession(std::string host, int16_t port);

  ~JanusSession();

  void Init();

  void Destory();

  //  void AttachVideoRoomPlugin();

  //  void DetachVideoRoomPlugin();

 private:
  void OnConnect();

  void OnMsg(const std::string& msg);

  void KeepAlive();

  void DoKeepAlive();

 private:
  std::unique_ptr<std::thread> io_thread_;
  boost::asio::io_context ioc_;
  std::shared_ptr<WebsocketTransport> ws_;
  int64_t session_id_{-1};
  std::unique_ptr<boost::asio::steady_timer> keep_alive_timer_;

  std::mutex mutex_;
  std::map<std::string, OnResponseCallback> response_callback_map_;
};
