#ifndef MOCKMESSAGELISTVIEW_H
#define MOCKMESSAGELISTVIEW_H

#include "interfaces/IMessageListView.h"

class MockMessageListView : public IMessageListView {
  public:
    virtual void setMessageModel(MessageModel* model) override {

    }

    virtual void scrollToBottom() override {

    }

    virtual int getMaximumMessageScrollBar() const override {

    }

    virtual int getMessageScrollBarValue() const override {

    }

    virtual void setMessageScrollBarValue(int value) override {

    }
};

#endif // MOCKMESSAGELISTVIEW_H
