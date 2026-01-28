#include <iostream>

#include "../metro/metro.h"
#include "../metro/server.h"
#include "../metro/middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

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
