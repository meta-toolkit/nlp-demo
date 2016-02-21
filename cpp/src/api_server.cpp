/**
 * @author Sean Massung
 */

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <event2/buffer.h>
#include <evhttp.h>

#include "cpptoml.h"
#include "nlp_demo.h"
#include "meta/logging/logger.h"
#include "meta/util/shim.h"
#include "simple_http_server.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    using namespace meta;
    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto server_table = config->get_table("server");
    auto address = *server_table->get_as<std::string>("address");
    auto port = *server_table->get_as<int64_t>("port");
    auto num_threads = *server_table->get_as<int64_t>("num-threads");

    LOG(info) << "Creating demo object..." << ENDLG;
    auto start = std::chrono::high_resolution_clock::now();
    auto demo = make_unique<nlp_demo>(*config);

    LOG(info) << "Done. ("
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - start)
                     .count()
              << "ms)" << ENDLG;

    void (*on_request)(evhttp_request*, void*)
        = [](evhttp_request* req, void* void_ptr)
    {
        auto request_str = simple_http_server::receive_request_string(req);

        nlp_demo* demo_ptr = (nlp_demo*)void_ptr;
        auto result = demo_ptr->analyze(request_str);

        simple_http_server::send_response_string(req, result);
    };

    simple_http_server srv{on_request, demo.get(), address, port, num_threads};
    srv.start();
}
