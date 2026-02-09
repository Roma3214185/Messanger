#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include <QListView>

class MessageModel;

class MessageListView : public QListView {
  Q_OBJECT
 public:
  using Callback = std::function<void()>;
  explicit MessageListView(QWidget *parent = nullptr);

  void setMessageModel(MessageModel *model);
  void scrollListToBottom();
  int getMaximumMessageScrollBar() const;
  int getMessageScrollBarValue() const;
  void setMessageScrollBarValue(int value);
  void preserveFocusWhile(MessageModel *message_model, Callback update_model);
  QModelIndex findIndexByMessageId(long long id);
  void scrollToMessage(const QModelIndex &index_to_scroll);
  void mousePressEvent(QMouseEvent *event) override;

  Q_SIGNALS:
  void scrollChanged(int value);
  void clickedWithEvent(QMouseEvent *event);
};

#endif  // MESSAGELISTVIEW_H
