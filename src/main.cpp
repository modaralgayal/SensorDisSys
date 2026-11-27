#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <string>
#include <sstream>
#include "vec_utils.h"
#include <nlohmann/json.hpp>
using nlohmann::json;

#define ASIO_STANDALONE
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
    std::set<std::string> m_peers_uri; // a set of peer URIs to connect to
    // a dummy set of weights for initial testing. Should be a separate class in the future.
    std::vector<double> m_weights;
    std::vector<double> running_sum_lambda; // Push sum of all weights that THIS node has sent to far. 
    double step_size = 0.05; // This is how much the difference incoming weights affect 
                            // the vector.


    json j; 
    
    // Node's own weights
    void generate_weights(int feature_count) {
        std::random_device rdev;
        std::mt19937 gen(rdev());
        std::normal_distribution<> dist(0.0, 1.0);
        for (int i = 0; i < feature_count; i++) {
            m_weights.push_back(dist(gen));
        }

        j["m_weights"] = m_weights;
    }


    std::string weights_as_string(std::vector<double> weights) {
        std::string payload = "[";
        for (size_t i = 0; i < weights.size(); i++) {
            payload+= std::to_string(weights[i]);
            if (i < weights.size() - 1) payload += ", ";
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
                    j["m_weights"] = m_weights;
                    std::string payload = j.dump(4);
                    LOG(INFO) << "Broadcasting to " << m_peers.size()
                              << " peers. Payload: " << payload;

                    for (Connection client : m_peers)
                        m_server.send(client, payload, websocketpp::frame::opcode::text);
                }
                this->broadcast(); // create infinite loop by recursing
        });
    }

public:
    explicit SwarmNode(std::set<std::string> peers = {}) : m_peers_uri(std::move(peers)) {
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

        auto on_message = [this](Connection sender, auto msg) {
            try {
                json parsed = json::parse(msg->get_payload());
                std::vector<double> payload =
                    parsed["m_weights"].get<std::vector<double>>();

                m_weights = calc_lambda_running_sum(payload, m_weights, step_size);
                LOG(INFO) << "Message received. New weights: "
                          << weights_as_string(m_weights);
            } catch (const std::exception& e) {
                LOG(ERROR) << "Failed to parse incoming message: " << e.what();
            }
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

        for (const std::string& peer_uri : m_peers_uri)
            send_message(peer_uri);

        broadcast();
        m_server.run();
    }
};

int main(const int argc, const char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    uint16_t port = (argc >= 2) ? (uint16_t)std::stoi(argv[1]) : 5000; // default to port 5000
    std::set<std::string> peers_uri;
    for (int i = 2; i < argc; i++)
        peers_uri.insert(argv[i]);

    SwarmNode node(std::move(peers_uri));

    node.start(port);

    return 0;
}
