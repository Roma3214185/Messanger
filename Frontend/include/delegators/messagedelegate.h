#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QDateTime>
#include <QFile>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

#include "MessageToken.h"
#include "managers/TokenManager.h"
#include "models/messagemodel.h"

struct ReactionHitBox {
  QRect rect;
  long long reaction_id;
};

using MessageId = long long;
using ReactionHitBoxes = std::vector<ReactionHitBox>;

class MessageDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  MessageDelegate(DataManager *data_manager, TokenManager *token_manager, QObject *parent = nullptr);

  void setDrawReactions(bool status) { draw_reactions = status; }
  void setDrawAnswerOn(bool status) { draw_answer_on = status; }
  void setSaveHitboxes(bool status) { draw_hit_boxes = status; }
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  std::optional<int> reactionAt(long long messageId, const QPoint &pos) const;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

 Q_SIGNALS:
  void unreadMessage(Message &message) const;  // todo: remove this signal

 private:
  void drawAll(QPainter *painter, const QStyleOptionViewItem &option, const Message &msg, const User &sender) const;
  [[nodiscard]] User extractSender(long long sender_id) const;
  void drawBackgroundState(QPainter *painter, const QRect &rect, const QStyleOptionViewItem &option,
                           bool is_mine) const;
  void drawAvatar(QPainter *painter, const QRect &rect, const QPixmap &avatar, bool is_mine) const;
  void drawUsername(QPainter *painter, const QRect &rect, const QString &username, bool is_mine) const;
  void drawText(QPainter *painter, const QRect &rect, const std::vector<MessageToken> &tokens, bool is_mine) const;
  void drawTimestamp(QPainter *painter, const QRect &rect, const QDateTime &timestamp, bool is_mine) const;
  void drawStatus(QPainter *painter, const QRect &rect, bool is_sended, int read_cnt, bool is_mine) const;
  void drawReactions(QPainter *painter, const QRect &rect, const std::unordered_map<long long, int> &reactions,
                     std::optional<int> my_reaction, long long message_id) const;
  void drawReadCounter(QPainter *painter, const QRect &rect, const int read_cnt, bool is_mine) const;
  QPixmap makeReactionIcon(const QString &imagePath, int count, std::optional<long long> my_reaction,
                           long long reaction_id) const;
  void addInRect(QPainter *painter, const QRect &rect, const QPixmap &icon, long long reaction_id, long long message_id,
                 int &reaction_of_set) const;

  void drawAnswerOnStatus(QPainter *painter, QRect &rect, const QColor &color, const Message &answer_on_message,
                          long long message_id) const;
  int calculateTextHeight(const QString &text, int textWidth, const QFont &font) const;

  DataManager *data_manager_;
  TokenManager *token_manager_;  // todo: can be delated
  mutable std::unordered_map<MessageId, ReactionHitBoxes> hit_boxes_by_message_;
  bool draw_reactions = true;
  bool draw_answer_on = true;
  bool draw_hit_boxes = true;
};

#endif  // CHATDELEGATE_H
