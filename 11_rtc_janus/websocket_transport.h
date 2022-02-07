#pragma once

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <thread>

class WebsocketTransport
    : public std::enable_shared_from_this<WebsocketTransport> {
 public:
  using OnConnectCallback = std::function<void()>;
  using OnMsgCallback = std::function<void(std::string msg)>;

  WebsocketTransport(boost::asio::io_context& ioc, std::string host,
                     int16_t port);

  WebsocketTransport(boost::asio::io_context& ioc, std::string host,
                     int16_t port, std::string subprotocol);

  ~WebsocketTransport();

  void Connect();

  void Disconnect();

  void SetConnectCallback(OnConnectCallback callback);

  void SetMsgCallback(OnMsgCallback callback);

  void SendMsg(std::string msg);

 private:
  void Loop();

  void Exit();

  void OnResolve(boost::beast::error_code ec,
                 boost::asio::ip::tcp::resolver::results_type results);

  void OnConnect(
      boost::beast::error_code ec,
      boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);

  void OnHandshake(boost::beast::error_code ec);

  void OnRead(boost::beast::error_code ec, std::size_t bytes_transferred);

  void OnWrite(boost::system::error_code ec, std::size_t bytes_transferred);

  void OnClose(boost::system::error_code ec);

  void DoRead();

  void SendDataInner(const std::shared_ptr<std::string const>& data);

 private:
  std::string host_;
  int16_t port_;
  std::string subprotocol_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
  OnConnectCallback connect_callback_;
  OnMsgCallback msg_callback_;
  std::vector<std::shared_ptr<std::string const>> send_msg_queue_;
  bool connected_{false};
};
