
#include "websocket_transport.h"

#include "rtc/rtc_base/logging.h"

static void Fail(boost::beast::error_code ec, const char* what) {
  RTC_LOG(LS_ERROR) << what << ":" << ec.message();
}

WebsocketTransport::WebsocketTransport(boost::asio::io_context& ioc,
                                       std::string host, int16_t port)
    : WebsocketTransport(ioc, std::move(host), port, "") {}

WebsocketTransport::WebsocketTransport(boost::asio::io_context& ioc,
                                       std::string host, int16_t port,
                                       std::string subprotocol)
    : host_(std::move(host)),
      port_(port),
      subprotocol_(std::move(subprotocol)),
      resolver_(boost::asio::make_strand(ioc)),
      ws_(boost::asio::make_strand(ioc)) {}

WebsocketTransport::~WebsocketTransport() { Disconnect(); }

void WebsocketTransport::SetConnectCallback(OnConnectCallback callback) {
  connect_callback_ = std::move(callback);
}

void WebsocketTransport::SetMsgCallback(OnMsgCallback callback) {
  msg_callback_ = std::move(callback);
}

void WebsocketTransport::SendMsg(std::string msg) {
  if (!connected_) {
    RTC_LOG(LS_WARNING) << "websocket not connect, cannot send msg";
    return;
  }
  auto data = std::make_shared<std::string const>(std::move(msg));
  boost::asio::post(
      ws_.get_executor(),
      std::bind(&WebsocketTransport::SendDataInner, shared_from_this(), data));
}

void WebsocketTransport::Connect() {
  if (connected_) {
    return;
  }
  resolver_.async_resolve(
      host_, std::to_string(port_),
      boost::beast::bind_front_handler(&WebsocketTransport::OnResolve,
                                       shared_from_this()));
}

void WebsocketTransport::Disconnect() {
  if (ws_.is_open()) {
    boost::asio::post(ws_.get_executor(), [this]() {
      if (!ws_.is_open()) {
        return;
      }
      ws_.async_close(boost::beast::websocket::close_code::normal,
                      boost::beast::bind_front_handler(
                          &WebsocketTransport::OnClose, shared_from_this()));
    });
  }
}

void WebsocketTransport::OnResolve(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type results) {
  if (ec) {
    return Fail(ec, "resolve");
  }

  // Set the timeout for the operation
  boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

  boost::beast::get_lowest_layer(ws_).async_connect(
      results, boost::beast::bind_front_handler(&WebsocketTransport::OnConnect,
                                                shared_from_this()));
}

void WebsocketTransport::OnConnect(
    boost::beast::error_code ec,
    boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
  if (ec) {
    return Fail(ec, "connect");
  }
  boost::beast::get_lowest_layer(ws_).expires_never();
  ws_.set_option(boost::beast::websocket::stream_base::timeout::suggested(
      boost::beast::role_type::client));
  if (!subprotocol_.empty()) {
    ws_.set_option(boost::beast::websocket::stream_base::decorator(
        [this](boost::beast::websocket::request_type& req) {
          req.set(boost::beast::http::field::sec_websocket_protocol,
                  subprotocol_);
        }));
  }
  ws_.async_handshake(
      host_, "/",
      boost::beast::bind_front_handler(&WebsocketTransport::OnHandshake,
                                       shared_from_this()));
}

void WebsocketTransport::OnHandshake(boost::beast::error_code ec) {
  if (ec) {
    return Fail(ec, "handshake");
  }
  connected_ = true;
  if (connect_callback_) {
    connect_callback_();
  }
  DoRead();
}

void WebsocketTransport::DoRead() {
  ws_.async_read(buffer_, boost::beast::bind_front_handler(
                              &WebsocketTransport::OnRead, shared_from_this()));
}

void WebsocketTransport::OnRead(boost::beast::error_code ec,
                                std::size_t /*bytes_transferred*/) {
  if (ec) {
    connected_ = false;
    return Fail(ec, "read");
  }

  if (msg_callback_) {
    msg_callback_(std::move(boost::beast::buffers_to_string(buffer_.data())));
  }

  buffer_.consume(buffer_.size());

  DoRead();
}

void WebsocketTransport::OnWrite(boost::system::error_code ec,
                                 std::size_t /*bytes_transferred*/) {
  if (ec) {
    connected_ = false;
    return Fail(ec, "write");
  }
  send_msg_queue_.erase(send_msg_queue_.begin());
  if (!send_msg_queue_.empty()) {
    ws_.async_write(boost::asio::buffer(*send_msg_queue_.front()),
                    boost::beast::bind_front_handler(
                        &WebsocketTransport::OnWrite, shared_from_this()));
  }
}

void WebsocketTransport::OnClose(boost::system::error_code ec) {
  if (ec) {
    return Fail(ec, "close");
  }
  connected_ = false;
  RTC_LOG(LS_INFO) << "websocket closed";
}

void WebsocketTransport::SendDataInner(
    const std::shared_ptr<std::string const>& data) {
  send_msg_queue_.push_back(data);
  if (send_msg_queue_.size() > 1) return;
  ws_.async_write(boost::asio::buffer(*send_msg_queue_.front()),
                  boost::beast::bind_front_handler(&WebsocketTransport::OnWrite,
                                                   shared_from_this()));
}
