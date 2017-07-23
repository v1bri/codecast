#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <iostream>

// This flag will let us turn off the io_service to demostrate how async method
// calls are executed.
DEFINE_bool(run_io_service, true, "Run the io_service object.");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("");
    gflags::SetVersionString("");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    boost::asio::io_service io_service;
    io_service.post([]() {
            std::cout << "Running async method" << std::endl;
        });

    if (FLAGS_run_io_service) {
        std::cout << "Before io_service::run" << std::endl;
        std::cout << "--------" << std::endl;
        io_service.run();
        std::cout << "--------" << std::endl;
        std::cout << "After io_service::run" << std::endl;
    }

    return 0;
}
