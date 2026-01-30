#include <iostream>
#include <fstream>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    // SSE (Server-Sent Events) streaming
    app.get("/stream/sse", [](Context& c) {
        c.res.stream([](auto write) {
            for (int i = 0; i < 3; ++i) {
                std::string data = "data: message " + std::to_string(i) + "\n\n";
                if (!write(data.c_str(), data.length())) return false;
            }
            return true;
        }, 0, "text/event-stream");
    });

    // Chunked text streaming
    app.get("/stream/chunks", [](Context& c) {
        c.res.stream([](auto write) {
            write("Hello ", 6);
            write("World", 5);
            return true;
        }, 0, "text/plain");
    });

    // Fixed length streaming
    app.get("/stream/fixed", [](Context& c) {
        std::string content = "Fixed length content here";
        c.res.stream([content](auto write) {
            return write(content.c_str(), content.length());
        }, content.length(), "text/plain");
    });

    // File serving (if test file exists)
    app.get("/file/test", [](Context& c) {
        // Create a temp file for testing
        std::ofstream ofs("/tmp/metro_test.txt");
        ofs << "Hello from file";
        ofs.close();
        
        c.res.file("/tmp/metro_test.txt", "text/plain");
    });

    Server server(app, 3007);
    server.listen();
}
