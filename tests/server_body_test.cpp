#include <iostream>

#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    // Echo plain text
    app.post("/echo/text", [](Context& c) {
        auto& data = c.req.text();
        if (data.empty()) {
            c.res.status(400).text("Empty text body");
            return;
        }
        c.res.text(data);
    });

    // Echo JSON
    app.post("/echo/json", [](Context& c) {
        auto& data = c.req.json();
        if (data.empty()) {
            c.res.status(400).text("Empty JSON body");
            return;
        }
        c.res.json(data);
    });

    // Echo form-urlencoded
    app.post("/echo/form", [](Context& c) {
        auto& data = c.req.form();
        if (data.empty()) {
            c.res.status(400).text("Empty form body");
            return;
        }
        c.res.json(data);
    });

    // Echo binary
    app.post("/echo/binary", [](Context& c) {
        auto& data = c.req.binary();
        if (data.empty()) {
            c.res.status(400).text("Empty binary body");
            return;
        }

        std::string body(data.begin(), data.end());
        c.res.text(body);
    });

    Server server(app, 3003);
    server.listen();
}
