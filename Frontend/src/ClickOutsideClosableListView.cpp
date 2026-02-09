#include "ui/clickoutsideclosablelistview.h"

#include <QTimer>

#include "dto/Message.h"
#include "ui/OutsideClickFilter.h"
#include "ui/mainwindow.h"

ClickOutsideClosableListView::ClickOutsideClosableListView(QWidget* parent) : QListView(parent) {
  setWindowFlags(Qt::WindowStaysOnTopHint | Qt::ToolTip);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setResizeMode(QListView::Adjust);
  setMovement(QListView::Static);
  setSpacing(6);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setAttribute(Qt::WA_Hover, false);
  setSelectionMode(QAbstractItemView::NoSelection);

  filter_ = new OutsideClickFilter(this);
  this->installEventFilter(filter_);

  if (auto mainWindow = qobject_cast<MainWindow*>(this->parent()); mainWindow != nullptr) {
    connect(mainWindow, &MainWindow::clickedOnScreenPosition, filter_, &OutsideClickFilter::checkClickOutside);
    connect(mainWindow, &MainWindow::screenGeometryChanged, this, &ClickOutsideClosableListView::update);
  }
}

void ClickOutsideClosableListView::showEvent(QShowEvent* event) {
  QListView::showEvent(event);
  update();
}

void ClickOutsideClosableListView::closeEvent(QCloseEvent* event) {
  QListView::closeEvent(event);
  call_close_callback();
}

void ClickOutsideClosableListView::addAcceptableClickableWidget(QWidget* widget) {
  if (filter_) {
    filter_->addAcceptableClickableWidget(widget);
  }
}

void ClickOutsideClosableListView::setModel(QAbstractItemModel* model) {
  QListView::setModel(model);
  connect(model, &QAbstractItemModel::rowsInserted, this, &ClickOutsideClosableListView::update);
  connect(model, &QAbstractItemModel::rowsRemoved, this, &ClickOutsideClosableListView::update);
  connect(model, &QAbstractItemModel::modelReset, this, &ClickOutsideClosableListView::update);
}

void ClickOutsideClosableListView::setUpdateCallback(EventCallback update_callback) {
  update_callback_ = std::move(update_callback);
}

void ClickOutsideClosableListView::setOnCloseCallback(EventCallback close_callback) {
  close_callback_ = std::move(close_callback);
}

void ClickOutsideClosableListView::call_close_callback() {
  if (close_callback_.has_value()) {
    QTimer::singleShot(0, [cbCopy = close_callback_.value()]() { cbCopy(); });
  }
}

void ClickOutsideClosableListView::update() {
  if (update_callback_.has_value()) {
    update_callback_.value()();
  }
}
