#include "OutsideClickFilter.h"

#include <QTimer>

namespace {

bool clickedInsideWidget(const QWidget *widget, QPointF globalPoint) {
  if (!widget) return false;

  QRectF globalRect(widget->mapToGlobal(QPoint(0, 0)), widget->size());
  return globalRect.contains(globalPoint);
}

}  // namespace

OutsideClickFilter::OutsideClickFilter(QWidget *popupWidget) : popup_(popupWidget) {
  clickable_widgets_.push_back(popup_);
}

bool OutsideClickFilter::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto *mouseEvent = static_cast<QMouseEvent *>(event);
    checkClickOutside(mouseEvent->globalPosition());
  }
  return QObject::eventFilter(obj, event);
}

void OutsideClickFilter::checkClickOutside(QPointF point) {
  if (!popup_) return;

  if (clickIsOutside(point)) {
    popup_->close();
    // popup_ = nullptr;
  }
}

void OutsideClickFilter::addAcceptableClickableWidget(QWidget *widget) { clickable_widgets_.push_back(widget); }

bool OutsideClickFilter::clickIsOutside(QPointF point) {
  for (auto &widget : clickable_widgets_) {
    if (clickedInsideWidget(widget, point)) return false;
  }
  return true;
}
