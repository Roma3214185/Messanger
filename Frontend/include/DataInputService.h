#ifndef DATAINPUTSERVICE_H
#define DATAINPUTSERVICE_H

#include <QFile>
#include <QList>
#include <QString>
#include <QTextStream>

#include "Debug_profiling.h"
#include "dto/SignUpRequest.h"

struct ValidationResult {
  bool valid{false};
  QString message;
};

struct Config {
  int kMinLenOfName = 4;
  int kMaxLenOfName = 64;

  int kMinPasswordLength = 8;
  int kMaxPasswordLength = 64;

  int kMinTagLength = 4;
  int kMaxTagLength = 32;

  int kMinEmailLocalPartLength = 4;
  int kMaxEmailLocalPartLength = 64;
  QString kConfigDomainsPath = "/Users/roma/QtProjects/Chat/Frontend/config/domains.txt";
  mutable QList<QString> kDomains;
};

struct CharConfig {
    bool lettersAllowed{false};
    bool numbersAllowed{false};
    bool spaceAllowed{false};
    QString specialCharactersAllowed;
};


namespace DataInputService {

ValidationResult nameValidDetailed(const QString &name, const Config &cfg = Config());
ValidationResult emailValidDetailed(const QString &email, const Config &cfg = Config());
ValidationResult passwordValidDetailed(const QString &password, const Config &cfg = Config());
ValidationResult tagValidDetailed(const QString &tag, const Config &cfg = Config());

ValidationResult validateRegistrationUserInput(const SignUpRequest &input, const Config &cfg = Config());
ValidationResult validateLoginUserInput(const LogInRequest &input, const Config &cfg = Config());

}  // namespace DataInputService

namespace DataInputService::details {

ValidationResult checkLocalPart(const QString &local, const Config &cfg = Config());
bool isValidChar(QChar el, const CharConfig& config);
ValidationResult checkDomainPart(const QString &domain, const Config &cfg = Config());

}  // namespace DataInputService::details

#endif  // DATAINPUTSERVICE_H
