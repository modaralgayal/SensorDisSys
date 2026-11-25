#include "swarm_node.h"
#include <glog/logging.h>


int main(const int argc, const char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    const char* host = std::getenv("HOSTNAME");
    std::string hostname = host ? host : "node-0";
    int id = std::stoi(hostname.substr(hostname.rfind('-') + 1));

    uint16_t port = (argc >= 2) ? (uint16_t)std::stoi(argv[1]) : 5000; // default to port 5000
    std::set<std::string> peer_uris(argv + 2, argv + argc);

    SwarmNode node(id,std::move(peer_uris));
    node.start(port);
    return 0;
}
