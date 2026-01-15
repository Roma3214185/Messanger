#ifndef MESSAGEACTIONPANEL_H
#define MESSAGEACTIONPANEL_H

#include <QObject>
#include "dto/Message.h"
#include <QVBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class FixedHeightDelegate : public QStyledItemDelegate {
  public:
    FixedHeightDelegate(int height, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), itemHeight(height) {}

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
    std::vector<ReactionInfo> reactions_;
  public:
    MessageActionPanel(const Message &msg, const std::vector<ReactionInfo>& reactions, QWidget *parent = nullptr)
        : QWidget(parent), msg_(msg), reactions_(reactions)
    {
      auto *layout = new QVBoxLayout(this);
      layout->setContentsMargins(4,4,4,4);
      layout->setSpacing(4);

      actionList_ = new QListView(this);
      actionList_->setViewMode(QListView::ListMode);
      actionList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      actionList_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      actionList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      QFont font;
      font.setPointSize(18);
      actionList_->setFont(font);
      int itemHeight = 24;
      actionList_->setItemDelegate(new FixedHeightDelegate(itemHeight, actionList_)); // 24 px per item

      actionModel_ = new QStringListModel({"Copy", "Edit", "Delete"}, this);
      actionList_->setModel(actionModel_);

      int itemCount = actionModel_->rowCount();
      int spacing = actionList_->spacing();
      int totalHeight = itemCount * itemHeight + (itemCount-1)*spacing + 2*actionList_->frameWidth();
      actionList_->setFixedHeight(totalHeight);

      layout->addWidget(actionList_);

      emojiGrid_ = new QListView(this);
      emojiGrid_->setViewMode(QListView::IconMode);
      emojiGrid_->setSpacing(6);
      emojiGrid_->setIconSize(QSize(18,18));
      emojiGrid_->setResizeMode(QListView::Adjust);
      emojiGrid_->setMovement(QListView::Static);
      emojiGrid_->setFixedHeight(36);

      emojiModel_ = new QStandardItemModel(this);
      loadEmojiReactions();
      emojiGrid_->setModel(emojiModel_);

      layout->addWidget(emojiGrid_);

      connect(actionList_, &QListView::clicked, this, &MessageActionPanel::onActionClicked);
      connect(emojiGrid_, &QListView::clicked, this, &MessageActionPanel::onEmojiClicked);
    }

  Q_SIGNALS:
    void copyClicked(const Message &msg);
    void editClicked(const Message &msg);
    void deleteClicked(const Message &msg);
    void reactionClicked(const Message &msg, long long emojiId);

  private:
    void loadEmojiReactions() {
      for (const auto &r : reactions_) {
        QStandardItem *item = new QStandardItem;
        QIcon icon(QString::fromStdString(r.image));
        if(icon.isNull()) continue;
        item->setIcon(icon);
        item->setData(r.id, Qt::UserRole + 1);
        emojiModel_->appendRow(item);
      }
    }

    void onActionClicked(const QModelIndex &index) {
      QString action = index.data().toString();
      if (action == "Copy") Q_EMIT copyClicked(msg_);
      else if (action == "Edit") Q_EMIT editClicked(msg_);
      else if (action == "Delete") Q_EMIT deleteClicked(msg_);
      else LOG_ERROR("Invalid action");
      this->close();
    }

    void onEmojiClicked(const QModelIndex &index) {
      long long emojiId = index.data(Qt::UserRole + 1).toInt();
      Q_EMIT reactionClicked(msg_, emojiId);
      this->close();
    }

    Message msg_;
    QListView *actionList_;
    QStringListModel *actionModel_;
    QListView *emojiGrid_;
    QStandardItemModel *emojiModel_;
};


#endif // MESSAGEACTIONPANEL_H
