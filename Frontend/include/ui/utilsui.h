#ifndef UTILS_UI_H
#define UTILS_UI_H

#include <QListView>
#include <QRect>
#include <QTextCursor>
#include <QWidget>

class ReactionInfo;

namespace utils::ui {

enum class Direction : uint8_t { Above, Below };

void updateViewVisibility(QListView *view, QWidget *anchor, Direction direction = Direction::Below,
                          int maxVisibleRows = 8, int itemsPerRow = 1);

void clearLayout(QLayout *layout);

void insert_emoji(QTextCursor &cursor, std::optional<ReactionInfo> img_info_opt, int size_of_image = 16,
                  const QString &default_value = " ");

QPixmap getPixmapFromPath(const QString &path);

}  // namespace utils::ui

#endif  // UTILS_UI_H
