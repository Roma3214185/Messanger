#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QListView>
#include <QObject>
#include <QScrollBar>
#include <QTimer>

#include "models/messagemodel.h"
#include "interfaces/IMessageListView.h"
#include "models/messagemodel.h"

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
    QListView::setModel(model);
  }

  void scrollToBottom() override {
    QTimer::singleShot(
        20, this, [this]() { setMessageScrollBarValue(getMaximumMessageScrollBar()); });
  }

  int getMaximumMessageScrollBar() const override { return this->verticalScrollBar()->maximum(); }

  int getMessageScrollBarValue() const override { return this->verticalScrollBar()->value(); }

  void setMessageScrollBarValue(int value) override { this->verticalScrollBar()->setValue(value); }

  void preserveFocusWhile(MessageModel* message_model, std::function<void()> updateModel) override {
    QModelIndex anchorIndex = indexAt(QPoint(0, 0));
    int anchorId = message_model->data(anchorIndex, MessageModel::MessageIdRole).toInt();
    int anchorY = visualRect(anchorIndex).top();

    updateModel();

    QModelIndex newAnchorIndex = message_model->indexFromId(anchorId);
    int newAnchorY = visualRect(newAnchorIndex).top();

    int delta = newAnchorY - anchorY;
    verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
  }
};

#endif  // MESSAGELISTVIEW_H
