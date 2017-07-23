#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <iostream>

using tcp = boost::asio::ip::tcp;

// Our resolver app turns a domain name and optional service name into an IP
// address and port.
DEFINE_string(domain, "", "Domain name to resolve.");
DEFINE_string(service, "", "Service name to resolve.");

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
    tcp::resolver r(io_service);

    tcp::resolver::query q(FLAGS_domain, FLAGS_service);
    r.async_resolve(
        q, [q](const boost::system::error_code& ec,
               tcp::resolver::iterator it) {
            // The io_service will call this lambda when the domain name is
            // fully resolved or an error occurs. Always handle the error first.
            if (ec) {
                LOG(ERROR) << "Unable to resolve " << q.host_name() << ": "
                           << ec.message();
                return;
            }

            do {
                std::cout << (it->service_name().empty() ?
                              it->host_name() :
                              it->host_name() + "," + it->service_name())
                          << " -> " << it->endpoint() << std::endl;
            } while (++it != tcp::resolver::iterator());
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
