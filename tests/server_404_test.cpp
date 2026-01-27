#include <iostream>

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

int main() {
    Metro app;
    app.use(MIDDLEWARE::logger());

    app.get("/only-get", [](Context& c) {
        c.res.text("GET OK");
    });

    Server server(app, 3004);
    server.listen();
}
