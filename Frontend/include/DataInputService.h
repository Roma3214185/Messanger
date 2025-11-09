#ifndef DATAINPUTSERVICE_H
#define DATAINPUTSERVICE_H

#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "Debug_profiling.h"
#include "dto/SignUpRequest.h"

struct ValidationResult {
    bool valid;
    QString message;
};

struct Config {
    size_t kMinLenOfName = 4;
    size_t kMaxLenOfName = 64;

    size_t kMinPasswordLength = 8;
    size_t kMaxPasswordLength = 64;

    size_t kMinTagLength = 4;
    size_t kMaxTagLength = 32;

    size_t kMinEmailLocalPartLength = 4;
    size_t kMaxEmailLocalPartLength = 64;
    QString kConfigDomainsRoad = "/Users/roma/QtProjects/Chat/Frontend/config/domains.txt";
};

namespace DataInputService {

ValidationResult nameValidDetailed(const QString& name, const Config& cfg = Config());
ValidationResult emailValidDetailed(const QString& email, const Config& cfg = Config());
ValidationResult passwordValidDetailed(const QString& password, const Config& cfg = Config());
ValidationResult tagValidDetailed(const QString& tag, const Config& cfg = Config());

ValidationResult validateRegistrationUserInput(const SignUpRequest& input, const Config& cfg = Config());
ValidationResult validateLoginUserInput(const LogInRequest& input, const Config& cfg = Config());

} // namespace DataInputService

#endif // DATAINPUTSERVICE_H

