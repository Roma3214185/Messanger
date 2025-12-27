#ifndef USER_H
#define USER_H

#include <QString>

struct User {
  QString email;
  QString tag;
  QString name;
  long long id;
  QString avatarPath = "/Users/roma/QtProjects/Chat/default_avatar.jpeg";

  bool checkInvariants() {
    return id > 0 //todo: what if message saved as offline, them id will == 0, state pattern??
           && !email.isEmpty()
           && !name.isEmpty()
           && !tag.isEmpty()
           && !avatarPath.isEmpty();
  }
};

#endif  // USER_H
