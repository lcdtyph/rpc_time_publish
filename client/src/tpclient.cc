#include <memory>
#include <sys/time.h>
#include <sys/errno.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include <ev.h>

#include "time_publish.pb.h"
#include "time_publish.grpc.pb.h"

DEFINE_string(server, "", "Server address");
DEFINE_int32(port, 50000, "Server port");
DEFINE_uint32(interval, 60, "Publish interval in secs");

class TimePublishClient {
public:
    TimePublishClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(timesync::Publisher::NewStub(channel)) {
    }

    bool PublishTime() {
        timesync::PublishTimeRequest request;
        timesync::PublishTimeReply reply;
        grpc::ClientContext context;

        struct timespec tv;
        int ret = clock_gettime(CLOCK_REALTIME, &tv);
        if (!ret) {
            LOG(ERROR) << "Failed to get time: " << errno;
            return false;
        }
        request.set_sec(tv.tv_sec);
        request.set_nsec(tv.tv_nsec);

        auto status = stub_->PublishTime(&context, request, &reply);

        if (!status.ok()) {
            LOG(ERROR) << "Rpc error: " << status.error_message();
            return false;
        }
        if (!reply.success()) {
            LOG(INFO) << "Publish failed: " << reply.error();
        }
        return reply.success();;
    }

private:
    std::unique_ptr<timesync::Publisher::Stub> stub_;
};

int main(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    std::string server_address = FLAGS_server + ":" + std::to_string(FLAGS_port);
    TimePublishClient client{grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())};

    auto loop = EV_DEFAULT;
    ev_timer timer;
    ev_timer_init(&timer,
        [](EV_P_ ev_timer *w, int revents) {
            auto client = static_cast<TimePublishClient *>(w->data);
            if (!client->PublishTime()) {
                LOG(ERROR) << "Publish failed";
            }
        },
        0., FLAGS_interval
    );
    timer.data = &client;
    ev_timer_start(loop, &timer);

    ev_run(loop, 0);

    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    return 0;
}

