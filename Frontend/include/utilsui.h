#ifndef UTILS_UI_H
#define UTILS_UI_H

#include <QListView>
#include <QWidget>
#include <QRect>

namespace utils {

enum class Direction : uint8_t { Above, Below };

inline void updateViewVisibility(QListView *view, QWidget *anchor,
                                Direction direction = Direction::Below,
                                int maxVisibleRows = 8,
                                int itemsPerRow = 1)
{
  if(!view) return;

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
  int frame = view->frameWidth()*2 + view->contentsMargins().top() + view->contentsMargins().bottom();
  int height = visibleRows * rowHeight + frame;

  view->setFixedHeight(height);
  view->setFixedWidth(anchor->width() * itemsPerRow);

  // horizontally center
  int x = anchor->mapToGlobal(QPoint(0,0)).x() + (anchor->width()/2) - (view->width()/2);

  QRect screenRect = anchor->screen()->availableGeometry();

  int y = [&]() {
    if(direction == Direction::Below) {
      const int belowY = anchor->mapToGlobal(QPoint(0, anchor->height())).y();
      return belowY + view->height() <= screenRect.bottom()
                 ? belowY // fits below
                 : qMax(screenRect.top(), anchor->mapToGlobal(QPoint(0,0)).y() - view->height()); // fallback above
    } else {
      const int aboveY = anchor->mapToGlobal(QPoint(0,0)).y() - view->height();
      return aboveY >= screenRect.top()
                 ? aboveY // fits above
                 : qMin(screenRect.bottom() - view->height(), anchor->mapToGlobal(QPoint(0, anchor->height())).y()); // fallback below
    }
  }();

  x = qMax(screenRect.left(), qMin(x, screenRect.right() - view->width()));

  view->move(x, y);
  view->show();
}

}  // namespace

#endif // UTILS_UI_H
