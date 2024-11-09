#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <deque>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Define quiz questions and answers
struct QuizQuestion {
    std::string question;
    std::string answer;
};

std::vector<QuizQuestion> quizQuestions = {
    {"What is 2 plus 2?", "four"},
    {"What is the capital of France?", "Paris"},
    {"What is the color of the sky?", "blue"}
};

class session : public std::enable_shared_from_this<session> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::deque<std::string> message_queue_;
    int currentQuestionIndex = 0;
    bool is_connected_ = false;

public:
    explicit session(tcp::socket&& socket)
        : ws_(std::move(socket)) {}

    void run() {
        std::cout << "Session started. Waiting for WebSocket handshake..." << std::endl;
        ws_.async_accept(beast::bind_front_handler(&session::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec) {
        if(ec) {
            fail(ec, "accept");
            return;
        }

        is_connected_ = true;
        std::cout << "WebSocket handshake successful. Connection established." << std::endl;
        
        send_question();
    }

    void do_read() {
        std::cout << "Waiting for client message..." << std::endl;
        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec == websocket::error::closed) {
            is_connected_ = false;
            std::cout << "Connection closed by client." << std::endl;
            return;
        }

        if(ec) {
            fail(ec, "read");
            return;
        }

        std::string userAnswer = beast::buffers_to_string(buffer_.data());
        std::cout << "Received answer from client: " << userAnswer << std::endl;
        buffer_.consume(buffer_.size());

        handle_answer(userAnswer);
    }

    void handle_answer(const std::string& answer) {
        std::cout << "Processing answer: " << answer << std::endl;
        std::string correctAnswer = quizQuestions[currentQuestionIndex].answer;
        std::string response;

        // Check if the user's answer is correct
        if (answer == correctAnswer) {
            response = "Correct!";
            currentQuestionIndex++;
            std::cout << "Answer is correct. Moving to next question." << std::endl;
        } else if (answer == "No response") {
            response = "No response received from client.";
            std::cout << "No response from client. Handling appropriately." << std::endl;
        } else {
            response = "Incorrect, try again.";
            std::cout << "Answer is incorrect. Sending retry message." << std::endl;
        }

        send_message(response);

        // Send the next question if there are more questions
        if (currentQuestionIndex < quizQuestions.size()) {
            send_question();
        } else {
            std::cout << "Quiz complete. Sending completion message." << std::endl;
            send_message("Quiz complete!");
        }
    }

    void send_question() {
        if (currentQuestionIndex < quizQuestions.size()) {
            std::string question = quizQuestions[currentQuestionIndex].question;
            std::cout << "Sending question: " << question << std::endl;
            send_message(question);
        } else {
            std::cout << "No more questions to send." << std::endl;
        }
    }

    void send_message(const std::string& message) {
        if (!is_connected_) {
            std::cout << "Cannot send message; client is disconnected." << std::endl;
            return;
        }

        std::cout << "Queueing message: " << message << std::endl;
        message_queue_.push_back(message);
        if (message_queue_.size() == 1) {
            std::cout << "Message queue has one item. Sending message immediately." << std::endl;
            do_write();
        }
    }

    void do_write() {
        if (message_queue_.empty()) {
            std::cout << "Message queue is empty; nothing to write." << std::endl;
            return;
        }

        ws_.text(ws_.got_text());
        std::cout << "Writing message to client: " << message_queue_.front() << std::endl;
        ws_.async_write(net::buffer(message_queue_.front()),
            beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec) {
            if (ec == websocket::error::closed) {
                is_connected_ = false;
                std::cout << "Connection closed during write." << std::endl;
                return;
            } else {
                fail(ec, "write");
                return;
            }
        }

        std::cout << "Message sent successfully. Removing from queue." << std::endl;
        message_queue_.pop_front();
        if (!message_queue_.empty()) {
            std::cout << "Queue has more messages. Sending next message." << std::endl;
            do_write();
        } else {
            std::cout << "Queue is now empty. Ready for next client message." << std::endl;
            do_read();
        }
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
        is_connected_ = false;
    }
};

class listener : public std::enable_shared_from_this<listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc) {
        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }

    void run() {
        std::cout << "Listener started. Waiting for client connections..." << std::endl;
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
            std::cout << "Client connected. Starting new session." << std::endl;
            std::make_shared<session>(std::move(socket))->run();
        }
        do_accept();
    }

    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
};

int main() {
    try {
        net::io_context ioc{1};
        auto endpoint = tcp::endpoint(tcp::v4(), 9001);
        std::make_shared<listener>(ioc, endpoint)->run();
        std::cout << "WebSocket server started on ws://0.0.0.0:9001" << std::endl;
        ioc.run();
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
