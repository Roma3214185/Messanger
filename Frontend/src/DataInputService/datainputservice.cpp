#include <QList>

#include "DataInputService/datainputservice.h"

namespace DataInputService {

static constexpr int kMinPasswordLength = 8;
static constexpr int kMaxPasswordLength = 22;
static constexpr int kMinTagLength = 4;
static constexpr int kMaxTagLength = 11;
static constexpr int kMinLenOfName = 4;
static constexpr int kMaxLenOfName = 20;
static const QString kEmailDomain = "@gmail.com";

bool nameValid(const QString& name) {
    return name.size() >= kMinLenOfName && name.size() <= kMaxLenOfName;
}

bool emailValid(const QString& login) {
    if (!login.endsWith(kEmailDomain))
        return false;

    const QString beforeDomain = login.left(login.size() - kEmailDomain.size());
    if (beforeDomain.isEmpty())
        return false;

    for (auto c : beforeDomain)
        if (!c.isLetterOrNumber())
            return false;

    return true;
}

bool passwordValidLength(const QString& password) {
    return password.size() >= kMinPasswordLength && password.size() <= kMaxPasswordLength;
}

bool passwordValidCharacters(const QString& password) {
    const QList<QChar> allowedSymbols = {'!', '$', '_', '+'};
    for (auto c : password)
        if (!c.isLetterOrNumber() && !allowedSymbols.contains(c))
            return false;

    return true;
}

bool passwordValid(const QString& password) {
    return passwordValidLength(password) && passwordValidCharacters(password);
}

bool tagValidCharacters(const QString& tag) {
    if (tag.isEmpty() || !tag.front().isLetterOrNumber())
        return false;

    QChar prev = 'a';
    for (auto c : tag) {
        if (c.isPunct()) {
            if (c != '_' || prev == '_')
                return false;
        } else if (!c.isLetterOrNumber()) {
            return false;
        }
        prev = c;
    }
    return true;
}

bool tagValid(const QString& tag) {
    return tagValidCharacters(tag)
    && tag.size() >= kMinTagLength
                        && tag.size() <= kMaxTagLength;
}

} // namespace DataInputService
