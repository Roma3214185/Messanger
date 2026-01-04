#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QListView>
#include <QObject>
#include <QScrollBar>
#include <QTimer>

#include "Debug_profiling.h"
#include "interfaces/IMessageListView.h"
#include "models/messagemodel.h"

class MessageListView : public IMessageListView {
  Q_OBJECT
 public:
  explicit MessageListView(QWidget* parent = nullptr) {
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
      if (value == 0) Q_EMIT scrollChanged(value);
    });

    QListView::setFocusPolicy(Qt::NoFocus);
    QListView::setSelectionMode(QAbstractItemView::NoSelection);
    this->setMinimumWidth(300);
    setAttribute(Qt::WA_Hover, false);
    setMouseTracking(false);
  }

  void setMessageModel(MessageModel* model) override { QListView::setModel(model); }

  void scrollToBottom() override {
    QTimer::singleShot(
        20, this, [this]() { setMessageScrollBarValue(getMaximumMessageScrollBar()); });
  }

  int getMaximumMessageScrollBar() const override { return this->verticalScrollBar()->maximum(); }

  int getMessageScrollBarValue() const override { return this->verticalScrollBar()->value(); }

  void setMessageScrollBarValue(int value) override { this->verticalScrollBar()->setValue(value); }

  void preserveFocusWhile(MessageModel*         message_model,
                          std::function<void()> update_model) override {
    const QModelIndex anchor_index = indexAt(QPoint(0, 0));
    const int anchor_id = message_model->data(anchor_index, MessageModel::MessageIdRole).toInt();
    const int anchor_y  = visualRect(anchor_index).top();

    update_model();

    const QModelIndex new_anchor_index = message_model->indexFromId(anchor_id);
    const int         new_anchor_y     = visualRect(new_anchor_index).top();

    const int delta = new_anchor_y - anchor_y;
    verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
  }
};

#endif  // MESSAGELISTVIEW_H
