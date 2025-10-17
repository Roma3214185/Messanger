#include <QApplication>
#include "src/MainWindow/mainwindow.h"
#include "src/Presenter/presenter.h"
#include <QUrl>
#include "src/Model/model.h"
#include "NetworkAccessManager/networkaccessmanager.h"
#include "headers/RedisClient.h"
#include "../../DebugProfiling/Debug_profiling.h"

// #ifdef ENABLE_TRACY
//     #include "../external/tracy/public/tracy/Tracy.hpp"
// #endif

int main(int argc, char *argv[])
{


    init_logger("Frontend");
    RedisClient redis("tcp://127.0.0.1:6379");
    QWebSocket socket;
    QApplication a(argc, argv);
    MainWindow w;
    QUrl url("http://localhost:8083");
    NetworkAccessManager manager;

    Model model(url, &manager, &redis, &socket);
    Presenter presenter(&w, &model);
    w.setPresenter(&presenter);
    w.show();
    return a.exec();
}
