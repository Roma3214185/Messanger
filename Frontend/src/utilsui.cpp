#include "ui/utilsui.h"

#include <QLayout>
#include <QListView>
#include <QRect>
#include <QScreen>
#include <QTextImageFormat>
#include <QWidget>

#include "entities/ReactionInfo.h"

namespace utils::ui {

void updateViewVisibility(QListView* view, QWidget* anchor, Direction direction, int max_visible_rows, int itemsPerRow) {
  if (!view) return;

  if (!view->model()) {
    view->hide();
    return;
  }

  const int rows = view->model()->rowCount();
  if (rows == 0) {
    view->hide();
    return;
  }

  const int row_height = view->sizeHintForRow(0);
  if (row_height <= 0) {
    view->hide();
    return;
  }

  const int visible_rows = qMin(rows, max_visible_rows);
  const int frame = view->frameWidth() * 2 + view->contentsMargins().top() + view->contentsMargins().bottom();
  const int height = visible_rows * row_height + frame;

  view->setFixedHeight(height);
  view->setFixedWidth(anchor->width() * itemsPerRow);

  // horizontally center
  int x = anchor->mapToGlobal(QPoint(0, 0)).x() + (anchor->width() / 2) - (view->width() / 2);

  QRect screenRect = anchor->screen()->availableGeometry();

  const int y = [&]() {
    if (direction == Direction::Below) {
      const int belowY = anchor->mapToGlobal(QPoint(0, anchor->height())).y();
      return belowY + view->height() <= screenRect.bottom()
                 ? belowY                                                                           // fits below
                 : qMax(screenRect.top(), anchor->mapToGlobal(QPoint(0, 0)).y() - view->height());  // fallback above
    } else {
      const int aboveY = anchor->mapToGlobal(QPoint(0, 0)).y() - view->height();
      return aboveY >= screenRect.top() ? aboveY  // fits above
                                        : qMin(screenRect.bottom() - view->height(),
                                               anchor->mapToGlobal(QPoint(0, anchor->height())).y());  // fallback below
    }
  }();

  x = qMax(screenRect.left(), qMin(x, screenRect.right() - view->width()));

  view->move(x, y);
  view->show();
}

void clearLayout(QLayout* layout) {
  if (!layout) return;
  while (auto item = layout->takeAt(0)) {
    if (auto w = item->widget()) {
      w->deleteLater();
    }
    delete item;
  }
}

void insert_emoji(QTextCursor& cursor, std::optional<ReactionInfo> img_info_opt, int size_of_image) {
  if (!img_info_opt.has_value()) {
    cursor.insertText(" ");
    return;
  }

  const ReactionInfo& info = img_info_opt.value();
  QTextImageFormat fmt;
  fmt.setName(QString::fromStdString(info.image));
  fmt.setWidth(size_of_image);
  fmt.setHeight(size_of_image);
  fmt.setProperty(QTextFormat::UserProperty, info.id);
  cursor.insertImage(fmt);
}

QPixmap getPixmapFromPath(const QString& path) {
  QPixmap avatar{path};
  return !avatar.isNull() ? avatar : QPixmap{"/Users/roma/QtProjects/Chat/default_avatar.jpeg"};
}

}  // namespace utils::ui
