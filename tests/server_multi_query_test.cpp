#include <iostream>

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    app.get("/tags", [](Context& c) {
        auto tags = c.req.queries("tag");

        if (tags.empty()) {
            c.res.status(400).text("No tags");
            return;
        }

        std::string out;
        for (auto t : tags) {
            if (!out.empty()) out += ",";
            out += std::string(t);
        }

        c.res.text(out);
    });

    Server server(app, 3002);
    server.listen();
}
