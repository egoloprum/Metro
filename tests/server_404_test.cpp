#include <iostream>

#include "../metro/metro.h"
#include "../metro/server.h"
#include "../metro/middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    app.get("/only-get", [](Context& c) {
        c.res.text("GET OK");
    });

    Server server(app, 3004);
    server.listen();
}
