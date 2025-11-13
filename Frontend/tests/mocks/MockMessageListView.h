#ifndef MOCKMESSAGELISTVIEW_H
#define MOCKMESSAGELISTVIEW_H

#include "interfaces/IMessageListView.h"

class MockMessageListView : public IMessageListView {
  public:
    MockMessageListView(int value, int max) : value_(value), max_(max) {}
    int value_;
    int max_;
    int call_scroll_to_bottom = 0;

    void setMessageModel(MessageModel* model) override {

    }

    void scrollToBottom() override {
      ++call_scroll_to_bottom;
    }

    int getMaximumMessageScrollBar() const override {
      return max_;
    }

    int getMessageScrollBarValue() const override {
      return value_;
    }

    void setMessageScrollBarValue(int value) override {

    }
};

#endif // MOCKMESSAGELISTVIEW_H
