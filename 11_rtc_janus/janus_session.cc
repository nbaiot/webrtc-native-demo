
#include "janus_session.h"

#include "rtc/rtc_base/helpers.h"
#include "rtc/rtc_base/logging.h"

#define RANDOM_STR_LENGTH 16
#define KEEPALIVE_INTERVAL_SECONDS 5

JanusSession::JanusSession(std::string host, int16_t port) {
  ws_ = std::make_shared<WebsocketTransport>(ioc_, std::move(host), port,
                                             "janus-protocol");
  ws_->SetConnectCallback([this]() { OnConnect(); });
  ws_->SetMsgCallback([this](std::string msg) { OnMsg(msg); });

  keep_alive_timer_ = std::make_unique<boost::asio::steady_timer>(ioc_);

  // must after create ws
  io_thread_ = std::make_unique<std::thread>([this]() { ioc_.run(); });
}

JanusSession::~JanusSession() {
  Destory();
  ioc_.stop();
  if (io_thread_->joinable()) {
    io_thread_->join();
  }
}

void JanusSession::Init() { ws_->Connect(); }

void JanusSession::Destory() {
  keep_alive_timer_->cancel();
  ws_->Disconnect();
}

void JanusSession::OnConnect() {
  RTC_LOG(LS_INFO) << ">>>>> connect janus";
  auto tsd = rtc::CreateRandomString(RANDOM_STR_LENGTH);
  {
    std::unique_lock<std::mutex> lk(mutex_);
    response_callback_map_[tsd] = [this](const nlohmann::json& res) {
      auto success = res.value("janus", "");
      if (success == "success") {
        session_id_ = res["data"]["id"];
        RTC_LOG(LS_INFO) << ">>>>> session_id:" << session_id_;
        KeepAlive();
      } else if (success == "error") {
        int32_t code = res["error"]["code"];
        std::string reason = res["error"]["reason"];
        RTC_LOG(LS_ERROR) << "create janus session failed, code:" << code
                          << ", reason:" << reason;
      }
    };
  }
  nlohmann::json create = {
      {"janus", "create"},
      {"transaction", tsd},
  };
  RTC_LOG(LS_INFO) << create.dump();
  ws_->SendMsg(create.dump());
}

void JanusSession::OnMsg(const std::string& msg) {
  RTC_LOG(LS_INFO) << ">>>>> receive msg:\n" << msg;
  auto message = nlohmann::json::parse(msg);
  std::string tsd = message.value("transaction", "");
  if (!tsd.empty()) {
    OnResponseCallback callback;
    {
      std::unique_lock<std::mutex> lk(mutex_);
      auto it = response_callback_map_.find(tsd);
      if (it == response_callback_map_.end()) {
        return;
      }
      callback = std::move(it->second);
      response_callback_map_.erase(it);
    }
    if (callback) {
      callback(message);
    }
  }
}

void JanusSession::KeepAlive() {
  keep_alive_timer_->expires_from_now(
      boost::asio::chrono::seconds(KEEPALIVE_INTERVAL_SECONDS));
  keep_alive_timer_->async_wait([this](const boost::system::error_code& error) {
    if (error) {
      return;
    }
    DoKeepAlive();
    KeepAlive();
  });
}

void JanusSession::DoKeepAlive() {
  if (session_id_ == -1) {
    return;
  }
  nlohmann::json keepalive = {
      {"janus", "keepalive"},
      {"session_id", session_id_},
      {"transaction", rtc::CreateRandomString(RANDOM_STR_LENGTH)},
  };
  RTC_LOG(LS_INFO) << keepalive.dump();
  ws_->SendMsg(keepalive.dump());
}
