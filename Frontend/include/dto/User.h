#ifndef USER_H
#define USER_H

#include <QString>
#include <string>

struct User {
  QString email;
  QString tag;
  QString name;
  long long id{0};
  QString avatarPath = "/Users/roma/QtProjects/Chat/images/default_avatar.jpg";

  bool checkInvariants() const {
    return id > 0 && !email.isEmpty() && !name.isEmpty() && !tag.isEmpty();  //&& !avatarPath.isEmpty();
  }

  std::string toString() const {
    std::string res;
    res += " | id = " + std::to_string(id);
    res += " | email = " + email.toStdString();
    res += " | tag = " + tag.toStdString();
    res += " | name = " + name.toStdString();
    res += " | avatar_path = " + avatarPath.toStdString();
    return res;
  }
};

#endif  // USER_H
