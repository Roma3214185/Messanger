#include <QApplication>
#include <QUrl>

#include "Debug_profiling.h"
#include "RealSocket.h"
#include "RedisClient.h"
#include "mainwindow.h"
#include "managers/datamanager.h"
#include "managers/networkaccessmanager.h"
#include "managers/Managers.h"
#include "managers/TokenManager.h"
#include "model.h"
#include "handlers/SocketHandlerRegistry.h"
#include "presenter.h"
#include "JsonService.h"
#include "usecases/DefaultUseCaseRepository.h"
#include "usecases/messageusecase.h"
#include "usecases/socketusecase.h"
#include "delegators/DelegatorsFactory.h"

int main(int argc, char *argv[]) {
  qRegisterMetaType<Message>("Message");
  initLogger("Frontend");
  RedisClient redis("tcp://127.0.0.1:6379");
  QWebSocket socket;
  RealSocket real_socket(&socket);
  QApplication a(argc, argv);
  QUrl url("http://localhost:8084");
  NetworkAccessManager network_manager;
  DataManager data_manager;
  TokenManager token_manager;
  JsonService json_service(&token_manager);

  auto chat_use_case = std::make_unique<ChatUseCase>(
      std::make_unique<ChatManager>(&json_service, &network_manager, url), &data_manager, &token_manager);

  auto message_use_case = std::make_unique<MessageUseCase>(
      &data_manager, std::make_unique<MessageManager>(&json_service, &network_manager, url), &token_manager);

  auto user_use_case = std::make_unique<UserUseCase>(
      &data_manager, std::make_unique<UserManager>(&json_service, &network_manager, url), &token_manager);

  auto session_use_case = std::make_unique<SessionUseCase>(
      std::make_unique<SessionManager>(&json_service, &network_manager, url));

  auto soket_use_case = std::make_unique<SocketUseCase>(
      std::make_unique<SocketManager>(&real_socket, url));

  DefaultUseCaseRepository use_case_repository{
      std::move(chat_use_case),
      std::move(message_use_case),
      std::move(user_use_case),
      std::move(session_use_case),
      std::move(soket_use_case)
  };

  Model model(&use_case_repository, &redis, &token_manager, &real_socket, &data_manager);
  DelegatorsFactory delegators_factory(&data_manager);

  MainWindow w(&model, &delegators_factory);
  Presenter presenter(&w, &model);
  presenter.initialHandlers(SocketHandlerRegistry::create(&model, &json_service));
  w.setPresenter(&presenter);
  w.initialise(); // presenter calls presenter_ initialise() ?
  //presenter.initialise();


  w.show();
  return a.exec();
}
