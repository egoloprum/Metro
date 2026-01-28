#include <iostream>

#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;
    
    App app;
    app.use(Middlewares::logger());

    app.get("/search", [](Context& c) {
        std::string q = std::string(c.req.query("q"));
        std::string page = std::string(c.req.query("page"));

        if (q.empty()) {
            c.res.status(400).text("Missing query");
            return;
        }

        c.res.text("q=" + q + ", page=" + (page.empty() ? "1" : page));
    });

    Server server(app, 3001);
    server.listen();
}
