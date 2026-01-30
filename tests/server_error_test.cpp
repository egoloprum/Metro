#include <iostream>
#include <string>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    Config config;
    config.setMaxBodySize(100); // Very small limit for testing 413
    
    App app;
    app.use(Middlewares::logger());

    // Payload too large (413)
    app.post("/upload", [](Context& c) {
        c.res.text("OK"); // Should never reach here if body > 100 bytes
    });

    // URI too long simulation (414) - via many query params
    app.get("/long-query", [](Context& c) {
        c.res.text("OK");
    });

    // Unsupported media type (415) - when Content-Type is missing on POST
    app.post("/strict", [](Context& c) {
        c.res.text("Received: " + c.req.text());
    });

    // Trigger internal server error (500)
    app.get("/panic", [](Context& c) {
        throw std::runtime_error("Intentional crash");
    });

    // Bad request via malformed JSON
    app.post("/json-strict", [](Context& c) {
        auto& j = c.req.json();
        c.res.json(j);
    });

    Server server(app, 3010);
    server.listen();
}
