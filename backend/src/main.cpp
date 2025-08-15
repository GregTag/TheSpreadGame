#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <cstdlib>
#include <memory>

#include "server.hpp"

int main(int argc, char* argv[]) {
    try {
        unsigned short port = 8080;
        if (argc > 1) {
            port = static_cast<unsigned short>(std::atoi(argv[1]));
        }

        {
            auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto logger = std::make_shared<spdlog::logger>("spread_server", std::move(sink));
            spdlog::set_default_logger(std::move(logger));
        }

        // Log pattern and level
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] [%t] %v");
        const char* lvl = std::getenv("LOG_LEVEL");
        if (lvl) {
            auto level = spdlog::level::from_str(lvl);
            spdlog::set_level(level);
        } else {
            spdlog::set_level(spdlog::level::info);
        }
        spdlog::flush_on(spdlog::level::info);

        spdlog::info("Starting Spread server on port {}", port);
        boost::asio::io_context ioc(std::thread::hardware_concurrency());
        Server server(ioc, port);
        server.start();
        ioc.run();
    } catch (const std::exception& ex) {
        spdlog::critical("Fatal error: {}", ex.what());
        return 1;
    }
    return 0;
}
