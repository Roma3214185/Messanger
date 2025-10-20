#include <QCoreApplication>

#include "Server/server.h"
#include "MessageManager/MessageManager.h"
#include "../GenericRepository/GenericReposiroty.h"
#include "../../DebugProfiling/Debug_profiling.h"
#include "NotificationManager/notificationmanager.h"


const int MESSAGE_PORT = 8082;

int main(int argc, char *argv[]) {
    init_logger("MessageService");
    QCoreApplication a(argc, argv);
    SQLiteDatabase bd;

    GenericRepository genRep(bd);
    MessageManager manager(genRep);
    NotificationManager notifManager(manager);

    Server server(MESSAGE_PORT, manager, notifManager);
    server.run();
    return a.exec();
}


