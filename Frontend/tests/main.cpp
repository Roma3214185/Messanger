#include <QApplication>
#include <catch2/catch_all.hpp>

#include "dto/Message.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  qRegisterMetaType<Message>("Message");
  return Catch::Session().run(argc, argv);
}
