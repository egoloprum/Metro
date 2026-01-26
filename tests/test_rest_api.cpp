#include <iostream> 

#include "../lib/metro.h"
#include "../lib/server.h"
#include "../lib/middleware.h"

struct User {
  std::string id = "123"; 
  std::string username = "user1";
};

int main() {
    Metro app;
    app.use(MIDDLEWARE::logger());

    User user; 

    app.get("/users/:id", [&user](Context& c) {
      std::string req_user_id = c.req.params("id");

      if (user.id == req_user_id) {
        c.res.body(user.username);
      }
      else {
        c.res.status(404).text("This user does not exist");
      }
    });


    app.put("/users/:id", [&user](Context& c) {
      std::string req_user_id = c.req.params("id");

      if (user.id == req_user_id) {
        user.username = "user2";
      }
      else {
        user.username = "user3";
      }

      c.res.status(200).text(user.username);
    });



    Server server(app, 3000);
    server.listen();
}
