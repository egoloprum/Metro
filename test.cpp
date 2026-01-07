#include <iostream> 

#include "lib/metro.h"
#include "lib/server.h"
#include "lib/middleware.h"

int main() {
    Metro app;

    app.use(MIDDLEWARE::logger());

    app.get("/", [](Context& c) {
        c.text("Hello Metro!");
    });

    app.get("/error", [](Context& c) {
        c.text("Hello Metro!", 500);
    });

    app.get("/users/:id", [](Context& c) {
        c.text("User id = " + c.params["id"]);
    });

    Server server(app, 3001);
    server.listen();
}
