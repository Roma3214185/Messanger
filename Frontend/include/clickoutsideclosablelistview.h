#ifndef CLICKOUTSIDECLOSABLELISTVIEW_H
#define CLICKOUTSIDECLOSABLELISTVIEW_H

#include <QListView>
#include <QWidget>
#include <QObject>

class OutsideClickFilter;

class ClickOutsideClosableListView final : public QListView {
  public:
    using EventCallback = std::function<void()>;

    ClickOutsideClosableListView(QWidget* parent);

    void addAcceptableClickableWidget(QWidget* widget);
    void setModel(QAbstractItemModel* model) override;
    void setUpdateCallback(EventCallback update_callback);
    void setOnCloseCallback(EventCallback close_callback);

  private:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void call_close_callback();
    inline void update();

    std::optional<EventCallback> update_callback_;
    std::optional<EventCallback> close_callback_;
    OutsideClickFilter* filter_;
};

#endif // CLICKOUTSIDECLOSABLELISTVIEW_H
