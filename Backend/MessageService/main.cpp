#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericReposiroty.h"
#include "MessageManager/MessageManager.h"
#include "NotificationManager/notificationmanager.h"
#include "Server/server.h"

const int MESSAGE_PORT = 8082;

int main(int argc, char *argv[]) {
    init_logger("MessageService");
    QCoreApplication a(argc, argv);
    SQLiteDatabase bd;
    RabbitMQClient mq("localhost", "guest", "guest");

    GenericRepository genRep(bd);
    MessageManager manager(genRep);
    //NotificationManager notifManager(manager);

    Server server(MESSAGE_PORT, manager, mq);
    server.run();
    return a.exec();
}


