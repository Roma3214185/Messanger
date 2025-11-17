#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QListView>
#include <QObject>
#include <QScrollBar>
#include <QTimer>

#include "models/messagemodel.h"
#include "interfaces/IMessageListView.h"

#include "Debug_profiling.h"

class MessageListView : public IMessageListView {
    Q_OBJECT
  public:
    explicit MessageListView(QWidget* parent = nullptr) {
      connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [=](int value){
        if(value == 0) Q_EMIT scrollChanged(value);
      });

    QListView::setFocusPolicy(Qt::NoFocus);
    QListView::setSelectionMode(QAbstractItemView::NoSelection);
    this->setMinimumWidth(300);
    setAttribute(Qt::WA_Hover, false);
    setMouseTracking(false);
  }

  void setMessageModel(MessageModel* model) override {
    LOG_INFO("---------------------- try to set model");
    QListView::setModel(model);
    LOG_INFO("---------------------- model is setted");

  }

  void scrollToBottom() override {
    QTimer::singleShot(
        20, this, [this]() { setMessageScrollBarValue(getMaximumMessageScrollBar()); });
  }

  int getMaximumMessageScrollBar() const override { return this->verticalScrollBar()->maximum(); }

  int getMessageScrollBarValue() const override { return this->verticalScrollBar()->value(); }

  void setMessageScrollBarValue(int value) override { this->verticalScrollBar()->setValue(value); }
};

#endif  // MESSAGELISTVIEW_H
