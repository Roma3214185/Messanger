#ifndef IMESSAGELISTVIEW_H
#define IMESSAGELISTVIEW_H

#include <QListView>
#include <QObject>

class MessageModel;

class IMessageListView : public QListView {
  Q_OBJECT
 public:
  explicit IMessageListView(QWidget *parent = nullptr);

  virtual void setMessageModel(MessageModel *model) = 0;

  virtual void scrollListToBottom() = 0;
  virtual int getMaximumMessageScrollBar() const = 0;
  virtual int getMessageScrollBarValue() const = 0;
  virtual void setMessageScrollBarValue(int value) = 0;
  virtual void preserveFocusWhile(MessageModel *message_model, std::function<void()> update_model) = 0;
  void mousePressEvent(QMouseEvent *event) override {
    Q_EMIT clickedWithEvent(event);
    QListView::mousePressEvent(event);
  }

 Q_SIGNALS:
  void scrollChanged(int value);
  void clickedWithEvent(QMouseEvent *event);
};

#endif  // IMESSAGELISTVIEW_H
