#include <iostream>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    // Route that returns JSON
    app.get("/api/data", [](Context& c) {
        c.res.json({{"status", "ok"}});
    });

    // Route that returns plain text
    app.get("/api/text", [](Context& c) {
        c.res.text("Plain text response");
    });

    // Route with explicit content type
    app.get("/api/xml", [](Context& c) {
        c.res.header("Content-Type", "application/xml");
        c.res.text("<root><item>1</item></root>");
    });

    Server server(app, 3009);
    server.listen();
}
