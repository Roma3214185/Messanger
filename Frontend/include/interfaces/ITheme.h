#ifndef ITHEME_H
#define ITHEME_H

#include <QString>

class ITheme {
 public:
  virtual QString getStyleSheet() = 0;
  virtual ~ITheme() = default;
};

class LightTheme : public ITheme {
 public:
  QString getStyleSheet() override;
};

class DarkTheme : public ITheme {
 public:
  QString getStyleSheet() override;
};

#endif  // ITHEME_H
