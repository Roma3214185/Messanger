#include "ui/MessageActionPanel.h"
#include "Debug_profiling.h"

MessageActionPanel::MessageActionPanel(const Message &msg, const std::vector<ReactionInfo> &reactions, QWidget *parent)
    : QWidget(parent), msg_(msg), reactions_(reactions) {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(Qt::Popup);
  setupActionList();
  setupEmojiiGrid();

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);
  layout->addWidget(actionList_);
  layout->addWidget(emojiGrid_);
}

void MessageActionPanel::setupActionList() {
    if(actionList_ != nullptr) return;
    actionList_ = new QListView(this);
    actionList_->setViewMode(QListView::ListMode);
    actionList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    actionList_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    actionList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QFont font;
    constexpr int kFontSize = 18;
    font.setPointSize(kFontSize);
    actionList_->setFont(font);

    constexpr int kItemHeight = 24;
    actionList_->setItemDelegate(new FixedHeightDelegate(kItemHeight, actionList_));
    actionModel_ = new QStringListModel({"Copy", "Edit", "Delete", "Answer"}, this);
    actionList_->setModel(actionModel_);

    const int item_count = actionModel_->rowCount();
    const int spacing = actionList_->spacing();
    const int total_height = item_count * kItemHeight + (item_count - 1) * spacing + 2 * actionList_->frameWidth();
    actionList_->setFixedHeight(total_height);
    connect(actionList_, &QListView::clicked, this, &MessageActionPanel::onActionClicked);
}

void MessageActionPanel::setupEmojiiGrid() {
    if(emojiGrid_ != nullptr) return;
    emojiGrid_ = new QListView(this);
    emojiGrid_->setViewMode(QListView::IconMode);
    emojiGrid_->setSpacing(6);
    emojiGrid_->setIconSize(QSize(18, 18));
    emojiGrid_->setResizeMode(QListView::Adjust);
    emojiGrid_->setMovement(QListView::Static);
    emojiGrid_->setFixedHeight(36);
    emojiModel_ = new QStandardItemModel(this);
    loadEmojiReactions();
    emojiGrid_->setModel(emojiModel_);
    connect(emojiGrid_, &QListView::clicked, this, &MessageActionPanel::onEmojiClicked);
}

void MessageActionPanel::loadEmojiReactions() {
  for (const auto &r : reactions_) {
    auto *item = new QStandardItem;
    QIcon icon(QString::fromStdString(r.image));
    if (icon.isNull()) continue;
    item->setIcon(icon);
    item->setData(r.id, Qt::UserRole + 1);
    emojiModel_->appendRow(item);
  }
}

void MessageActionPanel::onActionClicked(const QModelIndex &index) {
  QString action = index.data().toString();
  if (action == "Copy") {
    Q_EMIT copyClicked(msg_);
  } else if (action == "Edit") {
    Q_EMIT editClicked(msg_);
  } else if (action == "Delete") {
    Q_EMIT deleteClicked(msg_);
  } else if (action == "Answer") {
    Q_EMIT onAnswerClicked(msg_);
  } else {
    LOG_ERROR("Invalid action");
  }
  this->close();
}

void MessageActionPanel::onEmojiClicked(const QModelIndex &index) {
  long long emoji_id = index.data(Qt::UserRole + 1).toLongLong();
  Q_EMIT reactionClicked(msg_, emoji_id);
  this->close();
}
