#ifndef MESSAGEACTIONPANEL_H
#define MESSAGEACTIONPANEL_H

#include <QListView>
#include <QObject>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include "dto/Message.h"

class FixedHeightDelegate : public QStyledItemDelegate {
 public:
  FixedHeightDelegate(int height, QObject *parent = nullptr) : QStyledItemDelegate(parent), itemHeight(height) {}

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(itemHeight);
    return s;
  }

 private:
  int itemHeight;
};

class MessageActionPanel : public QWidget {
  Q_OBJECT
 public:
  MessageActionPanel(const Message &msg, const std::vector<ReactionInfo> &reactions, QWidget *parent = nullptr);

 Q_SIGNALS:
  void copyClicked(const Message &msg);
  void editClicked(const Message &msg);
  void deleteClicked(const Message &msg);
  void onAnswerClicked(const Message &msg);
  void reactionClicked(const Message &msg, long long emojiId);

 private:
  void loadEmojiReactions();
  void onActionClicked(const QModelIndex &index);
  void onEmojiClicked(const QModelIndex &index);
  void setupActionList();
  void setupEmojiiGrid();

  std::vector<ReactionInfo> reactions_;
  Message msg_;
  QListView *actionList_;
  QStringListModel *actionModel_;
  QListView *emojiGrid_;
  QStandardItemModel *emojiModel_;
};

#endif  // MESSAGEACTIONPANEL_H
