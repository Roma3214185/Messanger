#ifndef USER_H
#define USER_H

#include <QString>

struct User {
  QString email;
  QString tag;
  QString name;
  int id;
  QString avatarPath = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";
};

#endif  // USER_H
