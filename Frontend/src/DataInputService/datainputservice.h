#ifndef DATAINPUTSERVICE_H
#define DATAINPUTSERVICE_H

#include <QString>

namespace DataInputService {
namespace detail {
inline constexpr int kMinPasswordLength = 8;
inline constexpr int kMaxPasswordLength = 22;
inline constexpr int kMinTagLength = 4;
inline constexpr int kMaxTagLength = 11;
inline constexpr int kMinLenOfName = 4;
inline constexpr int kMaxLenOfName = 20;
inline constexpr int kMinEmailLocalPartLength = 4;
inline constexpr int kMaxEmailLocalPartLength = 26;
inline const QString kEmailDomain = "@gmail.com";
}  // namespace detail

bool emailValid(const QString& login);

bool passwordValid(const QString& password);
bool passwordValidLength(const QString& password);
bool passwordValidCharacters(const QString& password);

bool tagValid(const QString& tag);
bool tagValidCharacters(const QString& tag);

bool nameValid(const QString& name);

}  // namespace DataInputService

#endif  // DATAINPUTSERVICE_H
