#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QListView>
#include <QObject>
#include <QScrollBar>
#include <QTimer>

#include "MessageModel/messagemodel.h"

class MessageListView : public QListView {
  Q_OBJECT
 public:
  explicit MessageListView(QWidget* parent = nullptr) : QListView(parent) {
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this,
            [=](int value) {
              if (value == 0) {
                Q_EMIT scrollChanged(value);
              }
            });

    QListView::setFocusPolicy(Qt::NoFocus);
    QListView::setSelectionMode(QAbstractItemView::NoSelection);
    this->setMinimumWidth(300);
  }

  void setMessageModel(MessageModel* model) { QListView::setModel(model); }

  void scrollToBottom() {
    QTimer::singleShot(20, this, [this]() {
      setMessageScrollBarValue(getMaximumMessageScrollBar());
    });
  }

  int getMaximumMessageScrollBar() {
    return this->verticalScrollBar()->maximum();
  }

  int getMessageScrollBarValue() { return this->verticalScrollBar()->value(); }

  void setMessageScrollBarValue(int value) {
    this->verticalScrollBar()->setValue(value);
  }

 Q_SIGNALS:
  void scrollChanged(int value);
};

#endif  // MESSAGELISTVIEW_H
