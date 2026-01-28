#include <iostream>
#include <chrono> 
#include <thread> 

#include "../metro/metro.h"
#include "../metro/server.h"
#include "../metro/middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    app.get("/echo", [](Context& c) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        c.res.text("GET OK");
    });

    Server server(app, 3006);
    server.listen();
}
