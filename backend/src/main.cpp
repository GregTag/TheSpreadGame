#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>

#include "server.hpp"

int main(int argc, char* argv[]) {
    try {
        unsigned short port = 8080;
        if (argc > 1) {
            port = static_cast<unsigned short>(std::atoi(argv[1]));
        }
        std::cout << "Starting Spread server on port " << port << std::endl;
        boost::asio::io_context ioc(std::thread::hardware_concurrency());
        Server server(ioc, port);
        server.start();
        ioc.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
