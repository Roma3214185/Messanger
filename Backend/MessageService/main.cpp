#include <QCoreApplication>

#include "Server/server.h"
#include "MessageDataBase/messagedatabase.h"

const int MESSAGE_PORT = 8082;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    DataBase db;
    Server server(MESSAGE_PORT, db);
    server.run();


    return a.exec();
}


