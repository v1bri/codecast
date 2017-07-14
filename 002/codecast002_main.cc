// A quick Hello World in C++ will give us something to compile.

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <iostream>

// This command line flag will let us trigger a crash to demonstrate glog's
// stack trace.
DEFINE_bool(crash, false, "Crash the app");

void do_crash() {
    int* p = nullptr;
    std::cout << *p << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize gflags with the command line arguments. A usage message and
    // version string can be provided if it makes sense for your app.
    gflags::SetUsageMessage("Hello World in C++");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Initialize glog with the name of the app from the command line. This will
    // do the right things when the app is called from a symbolic link. The
    // failure signal handler will provide a helpful stack trace when our app
    // crashes.
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // Glog uses C++ stream operations to express logging statements. There are
    // many levels of log severity, from INFO all the way to FATAL (which will
    // automatically crash your app when executed).
    LOG(INFO) << "App start";

    std::cout << "Hello World!" << std::endl;

    if (FLAGS_crash) {
        do_crash();
    }

    LOG(INFO) << "App exit";
    return 0;
}
