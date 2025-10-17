#include <QCoreApplication>

#include "Server/server.h"
#include "MessageManager/MessageManager.h"
#include "../GenericRepository/GenericReposiroty.h"
#include "../../DebugProfiling/Debug_profiling.h"


const int MESSAGE_PORT = 8082;

int main(int argc, char *argv[]) {
    init_logger("MessageService");
    QCoreApplication a(argc, argv);
    //MessageStatusRepository statRep;
    SQLiteDatabase bd;
    GenericRepository genRep(bd);
    MessageManager manager(genRep);
    Server server(MESSAGE_PORT, manager);
    server.run();


    return a.exec();
}


