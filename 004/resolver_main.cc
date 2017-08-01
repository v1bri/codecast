#include <boost/asio.hpp>
#include <boost/asio/system_timer.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// We'll compactify some verbose code with these shorter names.
namespace asio = boost::asio;

using json = nlohmann::json;

// Domain and service names will come from an outside file.
DEFINE_string(sites_path, "",
              "Path to Newline Delimited JSON file of sites to crawl.");

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("Asio Bulk Resolver");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // The Newline Delimited JSON spec is available at http://ndjson.org. It is
    // "... a convenient format for storing or streaming structured data that
    // may be processed one record at a time." If everyone could just replace
    // their home grown CSV writers and parsers with ndjson the world would
    // be a happier place.
    // Anyway, we need to open the file and read it one line at a time.
    std::ifstream s(FLAGS_sites_path);
    if (!s.is_open()) {
        LOG(ERROR) << "Error opening ndjson file at " << FLAGS_sites_path;
        return 1;
    }

    std::vector<asio::ip::tcp::resolver::query> queries;
    while (!s.eof()) {
        std::string json_line;
        try {
            std::getline(s, json_line);
            s.peek();
        } catch (std::exception e) {
            LOG(ERROR) << "Error reading JSON: " << e.what();
            continue;
        }

        // Just like the last episode, we'll make the service name optional.
        try {
            json j = json::parse(json_line);
            if (j.find("service") == j.end()) {
                queries.push_back(
                    asio::ip::tcp::resolver::query(
                        j.at("host").get<std::string>(), ""));
            } else {
                queries.push_back(
                    asio::ip::tcp::resolver::query(
                        j.at("host").get<std::string>(),
                        j.at("service").get<std::string>()));
            }
        } catch (std::exception e) {
            LOG(ERROR) << "Error accessing JSON: " << e.what();
        }
    }

    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    asio::system_timer timer(io_service);

    // All target sites have been read into memory. Now to queue up queries with
    // the resolver.
    size_t queries_remaining = queries.size();
    for (const auto& q : queries) {
        // The `async_resolve()` API takes a const reference to the query
        // object. This means the Asio library might make a copy of the query,
        // or it might not. It would be nice to know for sure. Skimming through
        // the online docs unfortunately doesn't provide any clues.
        // StackOverflow suggests that arguments passed to Asio by const
        // reference are copied when needed, unless the docs say otherwise
        // (https://stackoverflow.com/questions/12799720). But it would be nice
        // to have an official spec that all Asio implementations can adhere to.
        resolver.async_resolve(
            q, [&timer, &queries_remaining, &q](
                const boost::system::error_code& ec,
                asio::ip::tcp::resolver::iterator it) {
                if (--queries_remaining == 0) {
                    if (timer.cancel()) {
                        LOG(INFO) << "All queries finished, cancelling timer";
                    }
                }

                if (ec) {
                    LOG(ERROR) << "Error resolving " << q.host_name() << ": "
                               << ec.message();
                    return;
                }

                // Print out all the endpoints associated with the domain and
                // service. The documentation guarantees at least one result
                // when successful.
                do {
                    std::cout << (it->service_name().empty() ?
                                  it->host_name() :
                                  it->host_name() + "," + it->service_name())
                              << " -> " << it->endpoint() << std::endl;
                } while (++it != boost::asio::ip::tcp::resolver::iterator());
            });
    }

    boost::system::error_code ec;
    timer.expires_from_now(std::chrono::seconds(3), ec);
    if (ec) {
        LOG(ERROR) << "Error setting timeout: " << ec.message();
        return 1;
    }
    timer.async_wait([&resolver](const boost::system::error_code& ec) {
            if (ec) {
                LOG(INFO) << "Timer is cancelled";
                return;
            }

            // Note that this will only cancel the queries which are still
            // queued. Executing queries will be allowed to finish. See the
            // Boost mailing list for more info
            // (https://lists.boost.org/boost-users/2007/06/28647.php).
            LOG(ERROR) << "Timeout expired, cancelling resolver";
            resolver.cancel();
        });

    io_service.run();

    return 0;
}
