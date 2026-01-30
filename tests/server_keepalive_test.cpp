#include <iostream>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    app.get("/keepalive-test", [](Context& c) {
        c.res.text("Connection should stay open");
    });

    app.get("/close-me", [](Context& c) {
        c.res.header("Connection", "close");
        c.res.text("Goodbye");
    });

    Server server(app, 3012);
    server.listen();
}
