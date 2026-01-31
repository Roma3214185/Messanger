#include "utilsui.h"

#include <QLayout>
#include <QListView>
#include <QRect>
#include <QScreen>
#include <QTextImageFormat>
#include <QWidget>

#include "entities/ReactionInfo.h"

namespace utils::ui {

void updateViewVisibility(QListView* view, QWidget* anchor, Direction direction, int maxVisibleRows, int itemsPerRow) {
  if (!view) return;

  if (!view->model()) {
    view->hide();
    return;
  }

  int rows = view->model()->rowCount();
  if (rows == 0) {
    view->hide();
    return;
  }

  int rowHeight = view->sizeHintForRow(0);
  if (rowHeight <= 0) {
    view->hide();
    return;
  }

  int visibleRows = qMin(rows, maxVisibleRows);
  int frame = view->frameWidth() * 2 + view->contentsMargins().top() + view->contentsMargins().bottom();
  int height = visibleRows * rowHeight + frame;

  view->setFixedHeight(height);
  view->setFixedWidth(anchor->width() * itemsPerRow);

  // horizontally center
  int x = anchor->mapToGlobal(QPoint(0, 0)).x() + (anchor->width() / 2) - (view->width() / 2);

  QRect screenRect = anchor->screen()->availableGeometry();

  int y = [&]() {
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

void insert_emoji(QTextCursor& cursor, std::optional<ReactionInfo> img_info_opt, int size_of_image,
                  const QString& default_value) {
  if (img_info_opt.has_value()) {
    const ReactionInfo& info = img_info_opt.value();
    QTextImageFormat fmt;
    fmt.setName(QString::fromStdString(info.image));
    fmt.setWidth(size_of_image);
    fmt.setHeight(size_of_image);
    fmt.setProperty(QTextFormat::UserProperty, info.id);
    cursor.insertImage(fmt);
  } else {
    cursor.insertText(default_value);
  }
}

}  // namespace utils::ui
