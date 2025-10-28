#include <QApplication>
#include <QUrl>

#include "Debug_profiling.h"
#include "MainWindow/mainwindow.h"
#include "Model/model.h"
#include "NetworkAccessManager/networkaccessmanager.h"
#include "Presenter/presenter.h"
#include "headers/RedisClient.h"

int main(int argc, char* argv[]) {
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
