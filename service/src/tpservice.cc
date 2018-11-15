#include <memory>
#include <sys/time.h>
#include <sys/errno.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>

#include "time_publish.pb.h"
#include "time_publish.grpc.pb.h"

DEFINE_int32(port, 50000, "Listening port");

class TimePublishServer {
public:
    ~TimePublishServer() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run() {
        std::string bind_addr = "[::]:" + std::to_string(FLAGS_port);

        grpc::ServerBuilder builder;
        builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);

        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        LOG(INFO) << "Server listening on " << bind_addr;

        HandleRpcs();
    }

private:
    class AsyncContext : public std::enable_shared_from_this<AsyncContext> {
    public:
        AsyncContext(timesync::Publisher::AsyncService *service, std::shared_ptr<grpc::ServerCompletionQueue> cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
        }

        void Register() {
            auto self{shared_from_this()};
            self_ = self;
            Process();
        }

        void Process() {
            auto self{shared_from_this()};
            struct timespec tv;
            int ret;

            switch(status_) {
            case CREATE:
                service_->RequestPublishTime(&ctx_, &request_, &responder_, cq_.get(), cq_.get(), this);
                status_ = PROCESS;
                break;

            case PROCESS:
                std::make_shared<AsyncContext>(service_, cq_)->Register();
                tv.tv_sec = request_.sec();
                tv.tv_nsec = request_.nsec();
                ret = clock_settime(CLOCK_REALTIME, &tv);
                if (!ret) {
                    LOG(ERROR) << "Set time error, errno: " << errno;
                    if (ret == EPERM) {
                        reply_.set_error("Permission Error");
                    } else {
                        reply_.set_error("Error number: " + std::to_string(ret));
                    }
                }
                reply_.set_success(ret == 0);
                responder_.Finish(reply_, grpc::Status::OK, this);
                status_ = FINISH;
                break;

            case FINISH:
                self_.reset();
                break;

            default:
                LOG(FATAL) << "Impossible to reach here";
                break;
            }
        }
    private:
        timesync::Publisher::AsyncService *service_;
        std::shared_ptr<grpc::ServerCompletionQueue> cq_;
        grpc::ServerContext ctx_;
        timesync::PublishTimeRequest request_;
        timesync::PublishTimeReply reply_;
        grpc::ServerAsyncResponseWriter<timesync::PublishTimeReply> responder_;

        enum AsyncStatus { CREATE, PROCESS, FINISH };
        AsyncStatus status_;

        std::shared_ptr<AsyncContext> self_;
    };

    void HandleRpcs() {
        void *tag;
        bool ok;

        std::make_shared<AsyncContext>(&service_, cq_)->Register();
        while (cq_->Next(&tag, &ok)) {
            if (ok) {
                static_cast<AsyncContext *>(tag)->Process();
            }
        }
        LOG(INFO) << "CompletionQueue shutdown";
    }

    std::shared_ptr<grpc::ServerCompletionQueue> cq_;
    timesync::Publisher::AsyncService service_;
    std::shared_ptr<grpc::Server> server_;
};

int main(int argc, char *argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    TimePublishServer server;
    server.Run();

    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    return 0;
}

