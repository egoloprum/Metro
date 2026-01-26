#include <iostream> 

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

int main() {
    Metro app;

    app.use(MIDDLEWARE::logger());

    app.get("/users/:id", [](Context& c) {
        std::string id = c.req.params("id");
        std::string q = c.req.query("q");          
        auto qs = c.req.queries("q");              
        std::string name = c.req.query("name");    

        std::cout << "queries: " << q << " " << name << std::endl;

        for (const std::string& query : qs)  {
          std::cout << "query: " << query << std::endl;
        }

        c.res.text("ok");
    });

    app.post("/echo", [](Context& c) {
        std::string body = c.req.body();
        c.res.status(201).body("Echo: " + body);
    });

    app.put("/echo", [](Context& c) {
        std::string body = c.req.body();
        c.res.status(201).body("Echo: " + body);
    });

    app.del("/echo", [](Context& c) {
        std::string body = c.req.body();
        c.res.status(201).body("Echo: " + body);
    });

    Server server(app, 3000);
    server.listen();
}
