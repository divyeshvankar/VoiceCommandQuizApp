#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// This class represents an individual WebSocket session
class session : public std::enable_shared_from_this<session> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    explicit session(tcp::socket&& socket)
        : ws_(std::move(socket)) {}

    void run() {
        // Accept the WebSocket handshake
        ws_.async_accept(beast::bind_front_handler(&session::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec) {
        if(ec)
            return fail(ec, "accept");

        // Start reading messages
        do_read();
    }

    void do_read() {
        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec == websocket::error::closed)
            return;

        if(ec)
            return fail(ec, "read");

        // Log received message
        std::cout << "Received message: " << beast::make_printable(buffer_.data()) << std::endl;

        // Echo back the message
        ws_.text(ws_.got_text());
        ws_.async_write(buffer_.data(), beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        buffer_.consume(buffer_.size());
        do_read();
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
};

// Accepts incoming connections and launches the session
class listener : public std::enable_shared_from_this<listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc) {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }

    void run() {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(net::make_strand(ioc_),
            beast::bind_front_handler(&listener::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket) {
        if(ec) {
            fail(ec, "accept");
        } else {
            // Launch a new session for the connected client
            std::make_shared<session>(std::move(socket))->run();
        }

        // Accept the next connection
        do_accept();
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
};

int main() {
    try {
        net::io_context ioc{1};

        // Create and launch a listening port
        auto endpoint = tcp::endpoint(tcp::v4(), 9001);
        std::make_shared<listener>(ioc, endpoint)->run();

        std::cout << "WebSocket server started on ws://127.0.0.1:9001" << std::endl;

        // Run the I/O context
        ioc.run();
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
