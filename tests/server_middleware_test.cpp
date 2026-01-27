#include <iostream>

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

int main() {
    Metro app;
    app.use(MIDDLEWARE::logger());

    app.use([](Context& c, Next next) {
        auto token = c.req.header("Authorization");
        if (!token || *token != "secret") {
            c.res.status(401).text("Unauthorized");
            return;
        }
        next();
    });

    app.get("/protected", [](Context& c) {
        c.res.text("Access granted");
    });

    Server server(app, 3005);
    server.listen();
}
