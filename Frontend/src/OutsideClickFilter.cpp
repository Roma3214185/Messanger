#include "OutsideClickFilter.h"

#include <QTimer>

namespace {

bool clickedInsideWidget(QWidget* widget, QPoint globalPoint) {
  if (!widget) return false;

  QRect globalRect = widget->rect();
  QPoint topLeft = widget->mapToGlobal(globalRect.topLeft());
  QRect mappedRect(topLeft, widget->size());

  return mappedRect.contains(globalPoint);
}

} // namespace

OutsideClickFilter::OutsideClickFilter(QWidget *popupWidget) : popup_(popupWidget) {
  clickable_widgets_.push_back(popup_);
}

bool OutsideClickFilter::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    checkClickOutside(mouseEvent->globalPos());
  }
  return QObject::eventFilter(obj, event);
}

void OutsideClickFilter::checkClickOutside(QPoint point) {
  if (!popup_)
    return;

  if(clickIsOutside(point)) {
    popup_->close();
    //popup_ = nullptr;
  }
}

void OutsideClickFilter::addAcceptableClickableWidget(QWidget* widget) {
  clickable_widgets_.push_back(widget);
}

bool OutsideClickFilter::clickIsOutside(QPoint point) {
  for(auto& widget: clickable_widgets_) {
    if(clickedInsideWidget(widget, point)) return false;
  }
  return true;
}
