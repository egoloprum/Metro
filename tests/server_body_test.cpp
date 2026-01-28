#include <iostream>

#include "../metro/metro.h"
#include "../metro/server.h"
#include "../metro/middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    app.post("/echo", [](Context& c) {
        auto& raw = c.req.body();            
        std::string body(raw.data(), raw.size());

        if (body.empty()) {
            c.res.status(400).text("Empty body");
            return;
        }

        c.res.text(body);
    });

    Server server(app, 3003);
    server.listen();
}
