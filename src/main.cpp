#include "websocketpp/common/connection_hdl.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <set>

#include <glog/logging.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

// define an alias for a server endpoint to receive peer connections asynchronously,
// as well as a socket connection and a socket connection comparison
using Server = websocketpp::server<websocketpp::config::asio>;
using Connection = websocketpp::connection_hdl;
using ConnectionCompare = std::owner_less<Connection>;

class SwarmNode {
private:
    // server endpoint to receive peer connections
    Server m_endpoint; 
    // a set tracking connections to peers
    std::set<Connection, ConnectionCompare> m_peers;
    // a dummy set of weights for initial testing. Should be a separate class in the future.
    std::vector<double> m_weights;

    void broadcast() {
        m_endpoint.set_timer(
            5000, [this](websocketpp::lib::error_code const& error) {
                if (error) {
                    LOG(ERROR) << "Broadcast timer failed!";
                    return;
                }

            if (m_peers.size() > 0) {
                std::string dummy_data = "[1, 2, 3]";
                LOG(INFO) << "Broadcasting to " << m_peers.size() << " peers.";

                for (Connection client : m_peers)
                    m_endpoint.send(client, dummy_data, websocketpp::frame::opcode::text);
            }

            this->broadcast(); // create infinite loop by recursing
        });
    }

public:
    SwarmNode() {

    }
}

int main(const int argc, const char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    int port = 5000; // default to port 9002
    if (argc == 2) port = std::stoi(argv[1]);

    SwarmNode node;
    node.start(port);

    return 0;
}
