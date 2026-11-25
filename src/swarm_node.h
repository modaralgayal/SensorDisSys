#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>
#include <cstdint>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>

// Define some aliases for easier readability
using Server = websocketpp::server<websocketpp::config::asio>;
using Client = websocketpp::client<websocketpp::config::asio_client>;
using Connection = websocketpp::connection_hdl;
using ConnectionCompare = std::owner_less<Connection>; // because Connection is a weak pointer under the hood, and sets don't like that

std::string weights_as_string(std::vector<double> weights);

class SwarmNode {
public:
    SwarmNode(int id, std::set<std::string> peer_uris);
    void start(uint16_t port);

private:
    struct Peer { Connection conn; bool outgoing; };

    void on_open(Connection conn, bool outgoing);
    void on_message(Connection conn, bool outgoing, const std::string& payload);
    void on_close(Connection conn);

    void connect_to(const std::string& uri);
    void send_to(const Peer& peer, const std::string& payload);
    void broadcast();
    std::string peer_ids() const;

    void generate_weights(int feature_count);

    int id;
    asio::io_context io_loop;
    Client client;
    Server server;
    std::map<int, Peer> peers; // map of peer ID to struct
    std::set<std::string> peer_uris; // a set of peer URIs to connect to
    std::vector<double> weights; // Node's weights
    double step_size = 0.5; // This is how much the difference incoming weights affect the vector.
};