#include "clickoutsideclosablelistview.h"

#include "OutsideClickFilter.h"
#include "mainwindow.h"

ClickOutsideClosableListView::ClickOutsideClosableListView(QWidget* parent) : QListView(parent) {
  //this->setAttribute(Qt::WA_DeleteOnClose);
  this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::ToolTip);
  this->setAttribute(Qt::WA_ShowWithoutActivating);
  this->setResizeMode(QListView::Adjust);
  this->setMovement(QListView::Static);
  this->setSpacing(6);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  this->setAttribute(Qt::WA_Hover, false);

  filter_ = new OutsideClickFilter(this);
  this->installEventFilter(filter_);

  if (auto mainWindow = qobject_cast<MainWindow*>(this->parent()); mainWindow != nullptr) {
    connect(mainWindow, &MainWindow::clickedOnPos, filter_, &OutsideClickFilter::checkClickOutside);
    connect(mainWindow, &MainWindow::geometryChanged, this, &ClickOutsideClosableListView::update);
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
  if(filter_) {
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
  if(close_callback_.has_value()) {
    QTimer::singleShot(0, [cbCopy = close_callback_.value()]() {
      cbCopy();
    });
  }
}

void ClickOutsideClosableListView::update() {
  if(update_callback_.has_value()) {
    update_callback_.value()();
  }
}
