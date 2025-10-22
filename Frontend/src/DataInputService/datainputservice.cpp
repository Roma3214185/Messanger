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

namespace {

bool firstElementIsLetterOrNumber(const QString& tag) {
    return !tag.isEmpty() && tag.front().isLetterOrNumber();
}

bool hasConsecutiveUnderscores(QChar ch, QChar prev) {
    return ch == '_' && prev != '_';
}

} // namespace

bool nameValid(const QString& name) {
    return name.size() >= kMinLenOfName && name.size() <= kMaxLenOfName;
}

bool emailValid(const QString& login) {
    if (!login.endsWith(kEmailDomain)) return false;

    const QString beforeDomain = login.left(login.size() - kEmailDomain.size());
    if (beforeDomain.isEmpty()) return false;

    for (const QChar& ch : beforeDomain) {
        if (!ch.isLetterOrNumber()) return false;
    }

    return true;
}

bool passwordValidLength(const QString& password) {
    return password.size() >= kMinPasswordLength && password.size() <= kMaxPasswordLength;
}

bool passwordValidCharacters(const QString& password) {
    const QList<QChar> allowedSymbols = { '!', '$', '_', '+' };

    for (const QChar& ch : password) {
        if (!ch.isLetterOrNumber() && !allowedSymbols.contains(ch)) return false;
    }

    return true;
}

bool passwordValid(const QString& password) {
    return passwordValidLength(password) && passwordValidCharacters(password);
}

bool tagValidCharacters(const QString& tag) {
    if (!firstElementIsLetterOrNumber(tag)) return false;

    QChar prevChar = QChar();
    for (const QChar& ch : tag) {
        if (ch.isLetterOrNumber() || (ch == '_' && !hasConsecutiveUnderscores(ch, prevChar))) {
            prevChar = ch;
        } else {
            return false;
        }
    }

    return true;
}

bool tagValid(const QString& tag) {
    return tagValidCharacters(tag)
            && tag.size() >= kMinTagLength
                && tag.size() <= kMaxTagLength;
}

} // namespace DataInputService
