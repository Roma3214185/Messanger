#ifndef MESSAGELISTVIEW_H
#define MESSAGELISTVIEW_H

#include "interfaces/IMessageListView.h"

class MessageModel;

class MessageListView : public IMessageListView {
  Q_OBJECT
 public:
  explicit MessageListView(QWidget *parent = nullptr);

  void setMessageModel(MessageModel *model) override;
  void scrollListToBottom() override;
  int getMaximumMessageScrollBar() const override;
  int getMessageScrollBarValue() const override;
  void setMessageScrollBarValue(int value) override;
  void preserveFocusWhile(MessageModel *message_model, std::function<void()> update_model) override;
};

#endif  // MESSAGELISTVIEW_H
