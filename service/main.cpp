#include "gflags/gflags.h"
#include "glog/logging.h"
#include "zrpc/zrpc.hpp"
#include "event.hpp"

DEFINE_int32(port, 40112, "server port");
DEFINE_int32(threads, 20, "server threads");
DEFINE_string(index, "/var/saedata/saedemo_data/aminer.index", "Serialized index.");
DEFINE_string(idmap, "/var/saedata/saedemo_data/aminer.idmap", "Serialized idmap.");
DEFINE_string(actdata, "/var/saedata/saedemo_data/aminer.act", "Serialized idmap.");

using namespace std;
using namespace zrpc;

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = true;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    LOG(INFO) << "AMiner Server Starting...";
    RpcServer* server = RpcServer::CreateServer(FLAGS_port, FLAGS_threads);

    LOG(INFO) << "Setting up services...";
    event::Trigger("init_authorservice", nullptr, server);
    event::Trigger("init_pubservice", nullptr, server);
    event::Trigger("init_confservice", nullptr, server);
    event::Trigger("init_actservice", nullptr, server);

    LOG(INFO) << "Trying to bringing our services up...";
    server->run();

    LOG(INFO) << "Exiting...";
    return 0;
}
