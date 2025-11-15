#ifndef SECONDTESTCONTROLLER_H
#define SECONDTESTCONTROLLER_H

#include "messageservice/controller.h"

class SecondTestController : public Controller {
  public:
    using Controller::Controller;
    using Controller::handleSaveMessage;
    using Controller::handleSaveMessageStatus;
};

#endif // SECONDTESTCONTROLLER_H
