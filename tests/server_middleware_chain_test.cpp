#include <iostream>
#include <sstream>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    
    // Request ID middleware (adds header)
    app.use([](Context& c, Next next) {
        c.res.header("X-Request-ID", "12345");
        next();
    });

    // Timing middleware (adds processing time header)
    app.use([](Context& c, Next next) {
        auto start = std::chrono::high_resolution_clock::now();
        next();
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        c.res.header("X-Processing-Time", std::to_string(ms));
    });

    // Conditional middleware that short-circuits
    app.use([](Context& c, Next next) {
        if (c.req.header("X-Block")) {
            c.res.status(403).text("Blocked by middleware");
            return; // Don't call next()
        }
        next();
    });

    // Route specific middleware via chaining (simulated)
    app.get("/chain-test", [](Context& c) {
        // Check if previous middlewares ran
        auto reqId = c.res._headers.find("X-Request-ID");
        c.res.json({
            {"middleware_applied", reqId != c.res._headers.end()},
            {"message", "Hello"}
        });
    });

    Server server(app, 3011);
    server.listen();
}
