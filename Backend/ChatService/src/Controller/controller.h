#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <crow/crow.h>

#include "database.h"

class Controller {
 public:
  Controller(crow::SimpleApp& app, DataBase& dataBase);
  void handleRoutes();

 private:
  void handleCreatingPrivateChat();
  void handleGetAllChats();
  void handleGetAllChatsById();
  void handleGetAllChatsMembers();

  crow::SimpleApp& app_;
  DataBase database_;
};

#endif  // CONTROLLER_H
