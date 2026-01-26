#include <iostream> 

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

int main() {
    Metro app;

    app.use(MIDDLEWARE::logger());

    app.get("/users/:id", [](Context& c) {
        std::string id = c.req.params("id");          
        std::string ua = c.req.header("User-Agent").value_or("unknown");
        std::string q = c.req.query("q");    
        
        std::cout << "q: " << q << std::endl;

        c.res.status(200).header("X-Test", "yes").text("User id: " + id);
    });

    app.post("/echo", [](Context& c) {
        std::string body = c.req.body();
        c.res.status(201).body("Echo: " + body);
    });

    Server server(app, 3000);
    server.listen();
}
