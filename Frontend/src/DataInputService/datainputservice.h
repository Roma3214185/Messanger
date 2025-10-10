#ifndef DATAINPUTSERVICE_H
#define DATAINPUTSERVICE_H

#include <QString>

namespace DataInputService {

bool emailValid(const QString& login);

bool passwordValid(const QString& password);
bool passwordValidLength(const QString& password);
bool passwordValidCharacters(const QString& password);

bool tagValid(const QString& tag);
bool tagValidCharacters(const QString& tag);

bool nameValid(const QString& name);

} // namespace DataInputService

#endif // DATAINPUTSERVICE_H
