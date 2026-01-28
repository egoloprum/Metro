#include <iostream>

#include "metro.h"
#include "server.h"
#include "middleware.h"

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
