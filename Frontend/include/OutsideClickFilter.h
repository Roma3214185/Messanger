#ifndef OUTSIDECLICKFILTER_H
#define OUTSIDECLICKFILTER_H

#include <QEvent>
#include <QMouseEvent>
#include <QObject>
#include <QWidget>

class OutsideClickFilter final : public QObject {
  Q_OBJECT
 public:
  using EventCallback = std::function<void()>;

  OutsideClickFilter(QWidget *popupWidget);
  void checkClickOutside(QPoint point);
  void addAcceptableClickableWidget(QWidget *widget);  // click on this widget will not close popupWidget

 private:
  bool eventFilter(QObject *obj, QEvent *event) override;
  bool clickIsOutside(QPoint point);

  QWidget *popup_;
  std::vector<QWidget *> clickable_widgets_;
};

#endif  // OUTSIDECLICKFILTER_H
