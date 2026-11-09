#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <string>

#include <glog/logging.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/connection_hdl.hpp>

// Define some aliases for easier readability
using Server = websocketpp::server<websocketpp::config::asio>;
using Client = websocketpp::client<websocketpp::config::asio_client>;
using Connection = websocketpp::connection_hdl;
using ConnectionCompare = std::owner_less<Connection>; // because Connection is a weak pointer under the hood, and sets don't like that

class SwarmNode {
private:
    asio::io_context m_io_loop; // shared single thread event loop
    Client m_client;
    Server m_server; 
    // a set tracking connections to peers
    std::set<Connection, ConnectionCompare> m_peers;
    // a dummy set of weights for initial testing. Should be a separate class in the future.
    std::vector<double> m_weights;

    void generate_weights(int feature_count) {
        std::random_device rdev;
        std::mt19937 gen(rdev());
        std::normal_distribution<> dist(0.0, 1.0);
        for (int i = 0; i < feature_count; i++) {
            m_weights.push_back(dist(gen));
        }
    }

    std::string weights_as_string() {
        std::string payload = "[";
        for (size_t i = 0; i < m_weights.size(); i++) {
            payload+= std::to_string(m_weights[i]);
            if (i < m_weights.size() - 1) payload += ", ";
        }
        payload += "]";
        return payload;
    }

    void broadcast() {
        m_server.set_timer(
            5000, [this](websocketpp::lib::error_code const& error) {
                if (error) {
                    LOG(ERROR) << "Broadcast timer failed!";
                    return;
                }
                if (m_peers.size() > 0) {
                    std::string payload = weights_as_string();
                    LOG(INFO) << "Broadcasting to " << m_peers.size() << " peers. " << "Broadcasted payload: " << payload;

                    for (Connection client : m_peers)
                        m_server.send(client, payload, websocketpp::frame::opcode::text);
                }
                this->broadcast(); // create infinite loop by recursing
        });
    }

public:
    SwarmNode() {
        generate_weights(3); // generate a vector of three weights for now
        m_server.clear_access_channels(websocketpp::log::alevel::all); // disable internal library logs for now
        m_client.clear_access_channels(websocketpp::log::alevel::all);

        m_server.init_asio(&m_io_loop);
        m_client.init_asio(&m_io_loop);

        auto on_open = [this](Connection peer) {
            LOG(INFO) << "Peer connection established to # peers: " << m_peers.size() + 1;
            m_peers.insert(peer);
        };

        auto on_close = [this](Connection peer) {
            LOG(INFO) << "Peer disconnected";
            m_peers.erase(peer);
        };

        auto on_message = [](Connection sender, auto msg) {
            LOG(INFO) << "Message received: " << msg->get_payload();
        };

        m_server.set_open_handler(on_open);
        m_server.set_close_handler(on_close);
        m_server.set_message_handler(on_message);

        m_client.set_open_handler(on_open);
        m_client.set_close_handler(on_close);
        m_client.set_message_handler(on_message);
    }

    void send_message(const std::string& uri) {
        websocketpp::lib::error_code error;
        Client::connection_ptr connection = m_client.get_connection(uri, error);

        if (error) {
            LOG(ERROR) << "Could not create connection: " << error.message();
            return;
        }

        LOG(INFO) << "Sending message to " << uri;
        m_client.connect(connection);
    }

    void start(uint16_t port) {
        LOG(INFO) << "Starting SwarmNode on port " << port;
        m_server.listen(port);
        m_server.start_accept();
        broadcast();
        m_server.run();
    }
};

int main(const int argc, const char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    uint16_t port = (argc >= 2) ? (uint16_t)std::stoi(argv[1]) : 5000; // default to port 5000
    std::string peer_uri = (argc >= 3) ? argv[2] : ""; // set connection target for testing

    SwarmNode node;

    if (!peer_uri.empty()) node.send_message(peer_uri);

    node.start(port);

    return 0;
}
