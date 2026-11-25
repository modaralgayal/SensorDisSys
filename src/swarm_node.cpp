#include "swarm_node.h"
#include "vec_utils.h"
#include <glog/logging.h>
#include <random>

SwarmNode::SwarmNode(int id, std::set<std::string> peer_uris)
    : id(id), peer_uris(std::move(peer_uris)) {
    generate_weights(10); // Initialize weights with 10 features
    server.clear_access_channels(websocketpp::log::alevel::all); // disable internal library logs for now
    client.clear_access_channels(websocketpp::log::alevel::all);

    server.init_asio(&io_loop);
    client.init_asio(&io_loop);

    server.set_open_handler([this](Connection conn) { on_open(conn, false);});
    server.set_close_handler([this](Connection conn) { on_close(conn); });
    server.set_message_handler([this](Connection conn, Server::message_ptr msg) { on_message(conn, false, msg->get_payload()); });

    client.set_open_handler([this](Connection conn) { on_open(conn, true); });
    client.set_close_handler([this](Connection conn) { on_close(conn); });
    client.set_message_handler([this](Connection conn, Client::message_ptr msg) { on_message(conn, true, msg->get_payload()); });
}

// Node's own weights
void SwarmNode::generate_weights(int feature_count) {
    std::random_device rdev;
    std::mt19937 gen(rdev());
    std::normal_distribution<> dist(0.0, 1.0);
    for (int i = 0; i < feature_count; i++) {
        weights.push_back(dist(gen));
    }
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

void SwarmNode::broadcast() {
    server.set_timer(
        5000, [this](websocketpp::lib::error_code const& error) {
            if (error) {
                LOG(ERROR) << "Broadcast timer failed!";
                return;
            }
            if (peers.size() > 0) {
                std::string payload = weights_as_string(weights);
                LOG(INFO) << "Broadcasting to " << peers.size() << " peers. " << "Broadcasted payload: " << payload;
            
                for (const auto& [peer_id, peer] : peers) 
                    send_to(peer, payload);
            }
            broadcast(); // create infinite loop by recursing
    });
}

void SwarmNode::connect_to(const std::string& uri) {
    websocketpp::lib::error_code error;
    Client::connection_ptr connection = client.get_connection(uri, error);

    if (error) {
        LOG(ERROR) << "Could not create connection: " << error.message();
        return;
    }

    LOG(INFO) << "Sending message to " << uri;
    client.connect(connection);
}

void SwarmNode::start(uint16_t port) {
    LOG(INFO) << "Starting SwarmNode on port " << port;
    server.listen(port);
    server.start_accept();

    for (const std::string& peer_uri : peer_uris)
        connect_to(peer_uri);

    broadcast();
    server.run();
}

void SwarmNode::on_open(Connection conn, bool outgoing) {
    std::string remote;
    if (outgoing) {
        remote = client.get_con_from_hdl(conn)->get_uri()->str();
    } else {
        remote = server.get_con_from_hdl(conn)->get_remote_endpoint();
    }
    LOG(INFO) << (outgoing ? "Outgoing to" : "Incoming from") << remote;
    Peer peer{conn, outgoing};
    send_to(peer, "hello " + std::to_string(id)); // using "hello" as a handshake
}

void SwarmNode::on_message(Connection conn, bool outgoing, const std::string& payload) {
    if (payload.rfind("hello ", 0) == 0) {
        int p_id = std::stoi(payload.substr(6));
        peers[p_id] = Peer{conn, outgoing};
        LOG(INFO) << "Peer " << p_id
                    << (outgoing ? " outgoing to." : " incoming from.");
        return;
    }
    
    std::string payload_as_string = payload;
    std::vector<double> payload_vec = convert_to_vec(payload_as_string);
    std::vector<double> lambda_weights = calc_lambda_running_sum(
        payload_vec, weights, step_size
    );
    weights = lambda_weights;
    LOG(INFO) << "Message received: " << payload_as_string;
    LOG(INFO) << "New weights: " << weights_as_string(lambda_weights);
}

void SwarmNode::on_close(Connection conn) {
    ConnectionCompare cmp;
    for (auto p = peers.begin(); p != peers.end(); ++p) {
        if (!cmp(p->second.conn, conn) && !cmp(conn, p->second.conn)) {
            LOG(INFO) << "Peer " << p->first << " disconnected";
            peers.erase(p);
            return;
        }
    }
}

void SwarmNode::send_to(const Peer& peer, const std::string& payload) {
    websocketpp::lib::error_code err;
    if (peer.outgoing)
        client.send(peer.conn, payload, websocketpp::frame::opcode::text, err);
    else
        server.send(peer.conn, payload, websocketpp::frame::opcode::text, err);
    if (err)
        LOG(ERROR) << "Sending failed: " << err.message();
}

std::string SwarmNode::peer_ids() const {
    std::string s = "[";
    for (const auto& [peer_id, peer] : peers) {
        s += (s.size() > 1 ? " " : "") + std::to_string(peer_id);
    }
    return s + "]";
}