#include "MessageListView.h"

#include <QListView>
#include <QObject>
#include <QScrollBar>
#include <QTimer>

#include "Debug_profiling.h"
#include "models/messagemodel.h"

MessageListView::MessageListView(QWidget *parent) {
  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
    if (value == 0) Q_EMIT scrollChanged(value);
  });

  QListView::setFocusPolicy(Qt::NoFocus);
  QListView::setSelectionMode(QAbstractItemView::NoSelection);
  this->setMinimumWidth(300);
  setAttribute(Qt::WA_Hover, false);
  setMouseTracking(false);
}

void MessageListView::setMessageModel(MessageModel *model) { QListView::setModel(model); }

void MessageListView::scrollListToBottom() {
  QTimer::singleShot(20, this, [this]() { setMessageScrollBarValue(getMaximumMessageScrollBar()); });
}

int MessageListView::getMaximumMessageScrollBar() const { return this->verticalScrollBar()->maximum(); }

int MessageListView::getMessageScrollBarValue() const { return this->verticalScrollBar()->value(); }

void MessageListView::setMessageScrollBarValue(int value) { this->verticalScrollBar()->setValue(value); }

void MessageListView::preserveFocusWhile(MessageModel *message_model, std::function<void()> update_model) {
  const QModelIndex anchor_index = indexAt(QPoint(0, 0));
  const int anchor_id = message_model->data(anchor_index, MessageModel::MessageIdRole).toInt();
  const int anchor_y = visualRect(anchor_index).top();

  update_model();

  const QModelIndex new_anchor_index = message_model->indexFromId(anchor_id);
  const int new_anchor_y = visualRect(new_anchor_index).top();

  const int delta = new_anchor_y - anchor_y;
  verticalScrollBar()->setValue(verticalScrollBar()->value() + delta);
}
