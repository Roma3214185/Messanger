#include <QApplication>
#include <QUrl>

#include "Debug_profiling.h"
#include "RealSocket.h"
#include "RedisClient.h"
#include "mainwindow.h"
#include "managers/datamanager.h"
#include "managers/networkaccessmanager.h"
#include "model.h"

int main(int argc, char *argv[]) {
  initLogger("Frontend");
  RedisClient redis("tcp://127.0.0.1:6379");
  QWebSocket socket;
  RealSocket real_socket(&socket);
  QApplication a(argc, argv);
  QUrl url("http://localhost:8084");
  NetworkAccessManager manager;
  DataManager data_manager;

  Model model(url, &manager, &redis, &real_socket, &data_manager);
  MainWindow w(&model);

  qRegisterMetaType<Message>("Message");

  w.show();
  return a.exec();
}
