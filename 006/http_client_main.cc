#include <boost/asio.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace asio = boost::asio;

using namespace std::placeholders;
using json = nlohmann::json;

// The HTTP URLs to fetch will come from a JSON file.
DEFINE_string(sites_path, "",
              "Path to newline delimited JSON file of sites to fetch.");

DEFINE_bool(print_body, false, "Print the HTTP GET response body.");

// All network and HTTP related operations for a given host and path will be
// handled by the HttpClient class.
class HttpClient {
public:
    HttpClient(asio::io_service& io_service, asio::ip::tcp::resolver& resolver,
               const std::string& host, const std::string& path)
        : host_(host), path_(path), resolver_(resolver), sock_(io_service) { }

    void Start() {
        // The client must start by resolving the hostname into an IP endpoint.
        // This will give us a destination for the TCP connection. We can
        // safely hard code the "http" service name.
        resolver_.async_resolve(
            asio::ip::tcp::resolver::query(host_, "http"),
            [this](const boost::system::error_code& ec,
                   asio::ip::tcp::resolver::iterator it) {
                if (ec) {
                    LOG(ERROR) << "Error resolving " << host_ << ": "
                               << ec.message();
                    return;
                }

                // For simplicity, we'll assume the first endpoint will always
                // be available.
                std::cout << host_ << ": resolved to " << it->endpoint()
                          << std::endl;
                do_connect(it->endpoint());
            });
    }

private:
    void do_connect(const asio::ip::tcp::endpoint& dest) {
        // Remember that the Asio library will make copies of parameters passed
        // by const reference, so it's ok to let the endpoint go out of scope
        // when this method returns.
        sock_.async_connect(
            dest, [this](const boost::system::error_code& ec) {
                if (ec) {
                    LOG(ERROR) << "Error connecting to " << host_ << ": "
                               << ec.message();
                    return;
                }

                std::cout << host_ << ": connected to "
                          << sock_.remote_endpoint() << std::endl;
                do_send_http_get();
            });
    }

    void do_send_http_get() {
        // At minimum, the remote server needs to know the path being fetched
        // and the host serving that path. The latter is required because a
        // single server often hosts multiple domains.
        request_ = "GET " + path_ + " HTTP/1.1\r\nHost: " + host_ + "\r\n\r\n";
        asio::async_write(
            sock_, asio::buffer(request_),
            [this](const boost::system::error_code& ec, std::size_t size) {
                if (ec) {
                    LOG(ERROR) << "Error sending GET " << ec;
                    return;
                }

                LOG(INFO) << host_ << ": sent " << size << " bytes";
                do_recv_http_get_header();
            });
    }

    void do_recv_http_get_header() {
        // Since HTTP/1.1 is a text based protocol, most of it is human readable
        // by design. Notice how the "double end of line" character sequence
        // ("\r\n\r\n") is used to delimit message sections.
        asio::async_read_until(
            sock_, response_, "\r\n\r\n",
            [this](const boost::system::error_code& ec, std::size_t size) {
                if (ec) {
                    LOG(ERROR) << "Error receiving GET header " << ec;
                    return;
                }

                LOG(INFO) << host_ << ": received " << size << ", streambuf "
                          << response_.size();

                // The asio::streambuf class can use multiple buffers
                // internally, so we need to use a special iterator to copy out
                // the header.
                std::string header(
                    asio::buffers_begin(response_.data()),
                    asio::buffers_begin(response_.data()) + size);
                response_.consume(size);

                std::cout << "----------" << std::endl << host_
                          << ": header length " << header.size() << std::endl
                          << header;

                // First we'll check for the explicit "Content-Length" length
                // field. This provides the exact body length in bytes.
                size_t pos = header.find("Content-Length: ");
                if (pos != std::string::npos) {
                    size_t len = std::strtoul(
                        header.c_str() + pos + sizeof("Content-Length: ") - 1,
                        nullptr, 10);
                    do_receive_http_get_body(len - response_.size());
                    return;
                }

                // The other alternative is a chunked transfer. There is a quick
                // way to determine the remaining length in this case.
                pos = header.find("Transfer-Encoding: chunked");
                if (pos != std::string::npos) {
                    do_receive_http_get_chunked_body();
                    return;
                }

                LOG(ERROR) << "Unknown body length";
            });
    }

    void do_receive_http_get_body(size_t len) {
        // For "Content-Length" we know exactly how many bytes are left to
        // receive.
        asio::async_read(
            sock_, response_, asio::transfer_exactly(len),
            std::bind(&HttpClient::handle_http_get_body, this, _1, _2));
    }

    void do_receive_http_get_chunked_body() {
        // For chunked transfers the final body chunk will be terminated by
        // another "double end of line" delimiter.
        asio::async_read_until(
            sock_, response_, "\r\n\r\n",
            std::bind(&HttpClient::handle_http_get_body, this, _1, _2));
    }

    void handle_http_get_body(const boost::system::error_code& ec,
                              std::size_t size) {
        if (ec) {
            LOG(ERROR) << "Error receiving GET body " << ec;
            return;
        }

        LOG(INFO) << host_ << ": received " << size << ", streambuf "
                  << response_.size();

        // We can finally consume the body and print it out if desired.
        const auto& data = response_.data();
        std::string body(asio::buffers_begin(data), asio::buffers_end(data));
        response_.consume(size);

        std::cout << "----------" << std::endl << host_ << ": body length "
                  << body.size() << std::endl;
        if (FLAGS_print_body) {
            std::cout << body;
        }
    }

    const std::string host_;
    const std::string path_;

    asio::ip::tcp::resolver& resolver_;
    asio::ip::tcp::socket sock_;

    std::string request_;
    asio::streambuf response_;
};

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("Asio HTTP client");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // Since we're trying to max out our "idiomatic C++" stats, we'll read the
    // JSON file with an ifstream object instead of the POSIX file API.
    std::ifstream s(FLAGS_sites_path);
    if (!s.is_open()) {
        LOG(ERROR) << "Error opening ndjson file at " << FLAGS_sites_path;
        return 1;
    }

    asio::io_service io_service;
    asio::ip::tcp::resolver resolver(io_service);
    std::vector<std::unique_ptr<HttpClient>> clients;

    // Loop over all entries in the file, skipping over errors with a warning
    // to the user.
    while (!s.eof()) {
        std::string json_line;
        try {
            std::getline(s, json_line);
            s.peek();
        } catch (std::exception e) {
            LOG(WARNING) << "Error reading JSON line: " << e.what();
            continue;
        }

        // Each line should parse as a complete JSON object containing our
        // two expected fields: "host" and "path".
        try {
            json j = json::parse(json_line);
            std::cout << j.at("host").get<std::string>() << ": fetching "
                      << j.at("path").get<std::string>() << std::endl;

            // We'll create a new HttpClient for each entry in the sites file.
            // Clearly there's plenty of room for optimization here.
            std::unique_ptr<HttpClient> c(
                new HttpClient(
                    io_service, resolver, j.at("host").get<std::string>(),
                    j.at("path").get<std::string>()));
            c->Start();

            clients.push_back(std::move(c));
        } catch(std::exception e) {
            LOG(WARNING) << "Error accessing JSON: " << e.what();
        }
    }

    io_service.run();

    return 0;
}
