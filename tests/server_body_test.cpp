#include <iostream>

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

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
