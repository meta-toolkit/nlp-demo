/**
 * @see
 * http://kukuruku.co/hub/cpp/lightweight-http-server-in-less-than-40-lines-on-libevent-and-c-11
 */

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include <event2/buffer.h>
#include <evhttp.h>

#include "cpptoml.h"
#include "nlp_demo.h"
#include "meta/logging/logger.h"
#include "meta/util/shim.h"

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

    LOG(info) << "Starting API server: " << address << ":" << port << ENDLG;
    start = std::chrono::high_resolution_clock::now();

    try
    {
        void (*on_request)(evhttp_request*, void*)
            = [](evhttp_request* req, void* void_ptr)
        {
            auto* out_buf = evhttp_request_get_output_buffer(req);
            if (!out_buf)
            {
                LOG(error) << "Couldn't get_output_buffer for request" << ENDLG;
                return;
            }

            struct evbuffer* in_evb = evhttp_request_get_input_buffer(req);
            std::string data(evbuffer_get_length(in_evb), ' ');
            evbuffer_copyout(in_evb, &data[0], data.size());

            nlp_demo* demo_ptr = (nlp_demo*) void_ptr;
            auto result = demo_ptr->analyze(data);
            evbuffer_add(out_buf, &result[0], result.size());
            evhttp_send_reply(req, HTTP_OK, "", out_buf);
        };

        std::exception_ptr init_except;
        std::atomic<bool> running{true};
        evutil_socket_t socket = -1;
        auto thread_fun = [&]()
        {
            try
            {
                std::unique_ptr<event_base, decltype(&event_base_free)>
                    event_base(event_base_new(), &event_base_free);
                if (!event_base)
                    throw std::runtime_error{"Failed to create new event_base"};
                std::unique_ptr<evhttp, decltype(&evhttp_free)> ev_http(
                    evhttp_new(event_base.get()), &evhttp_free);
                if (!ev_http)
                    throw std::runtime_error{"Failed to create new evhttp"};
                evhttp_set_gencb(ev_http.get(), on_request, demo.get());
                if (socket == -1)
                {
                    auto* bound_sock = evhttp_bind_socket_with_handle(
                        ev_http.get(), address.c_str(), port);
                    if (!bound_sock)
                        throw std::runtime_error{
                            "Failed to bind server socket"};
                    if ((socket = evhttp_bound_socket_get_fd(bound_sock)) == -1)
                        throw std::runtime_error{
                            "Failed to get server socket for next instance"};
                }
                else
                {
                    if (evhttp_accept_socket(ev_http.get(), socket) == -1)
                        throw std::runtime_error{
                            "Failed to bind server socket for new instance"};
                }
                while (running)
                {
                    event_base_loop(event_base.get(), EVLOOP_NONBLOCK);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            catch (...)
            {
                init_except = std::current_exception();
            }
        };
        auto deleter = [&](std::thread* t)
        {
            running = false;
            t->join();
            delete t;
        };
        using thread_ptr = std::unique_ptr<std::thread, decltype(deleter)>;
        std::vector<thread_ptr> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            thread_ptr thread{new std::thread(thread_fun), deleter};
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            if (init_except != std::exception_ptr())
            {
                running = false;
                std::rethrow_exception(init_except);
            }
            threads.emplace_back(std::move(thread));
        }
        LOG(info) << "Done. ("
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - start)
                         .count()
                  << "ms)" << ENDLG;
        std::cin.get();
        running = false;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
