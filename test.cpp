#include <iostream> 

#include "lib/metro.h"
#include "lib/server.h"

int main() {
    Metro app;

    app.use([](Context& c) {
        std::cout << c.method << " " << c.path << "\n";
    });

    app.get("/", [](Context& c) {
        c.text("Hello from C++ Hono!");
    });

    app.get("/users/:id", [](Context& c) {
        c.text("User id = " + c.params["id"]);
    });

    Server server(app, 3000);
    server.listen();
}
