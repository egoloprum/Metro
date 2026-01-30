#include <iostream>
#include "metro.h"
#include "server.h"
#include "middleware.h"

int main() {
    using namespace Metro;

    App app;
    app.use(Middlewares::logger());

    // Single path parameter
    app.get("/users/:id", [](Context& c) {
        auto id = c.req.params("id");
        c.res.json({{"user_id", id}});
    });

    // Multiple path parameters
    app.get("/users/:userId/posts/:postId", [](Context& c) {
        c.res.json({
            {"user", c.req.params("userId")},
            {"post", c.req.params("postId")}
        });
    });

    // Mixed route: static and param
    app.get("/api/v1/items/:itemId", [](Context& c) {
        c.res.text("Item: " + c.req.params("itemId"));
    });

    // Optional: param at root level
    app.get("/:slug", [](Context& c) {
        c.res.text("Slug: " + c.req.params("slug"));
    });

    // Param with query combination
    app.get("/search/:category", [](Context& c) {
        auto q = c.req.query("q");
        c.res.json({
            {"category", c.req.params("category")},
            {"query", q.empty() ? "all" : q}
        });
    });

    Server server(app, 3008);
    server.listen();
}
