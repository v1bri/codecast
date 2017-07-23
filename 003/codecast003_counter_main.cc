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

    size_t count = 0;
    std::function<void()> count_fn = [&count, &io_service, &count_fn]() {
        std::cout << "Count is " << ++count << std::endl;
        if (count < 10) {
            // Our example handler posts itself again to keep the loop going. A
            // real app would usually make another async call at this point
            // instead (ex: async_send, async_receive).
            io_service.post(count_fn);
        }
    };

    // Note that our example runs everything in the same thread (and function). So
    // the captured references to count, io_service and count_fn remain in scope
    // and valid for the duration of all async operations. You will need to be
    // careful with object lifetimes, references and pointers in your own apps.
    io_service.post(count_fn);

    if (FLAGS_run_io_service) {
        std::cout << "Before io_service::run" << std::endl;
        std::cout << "--------" << std::endl;
        io_service.run();
        std::cout << "--------" << std::endl;
        std::cout << "After io_service::run" << std::endl;
    }

    return 0;
}
