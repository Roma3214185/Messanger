#include "DataInputService.h"

#include <QChar>
#include <QLocale>
#include <QMutex>
#include <QString>
#include <algorithm>

namespace DataInputService {

void loadDomains(const Config &cfg) {
  if (!cfg.kDomains.isEmpty()) {
    return;
  }

  QFile file(cfg.kConfigDomainsPath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Cannot open domains file:" << cfg.kConfigDomainsPath;
    return;
  }

  QTextStream in(&file);
  while (!in.atEnd()) {
    if (QString line = in.readLine().trimmed(); !line.isEmpty()) {
      cfg.kDomains.append(line);
    }
  }
}

bool domainIsAllowedList(const QString &domain, const Config &cfg) {
  loadDomains(cfg);
  const QString normalized = domain.toLower();
  return cfg.kDomains.contains(normalized);
}

ValidationResult nameValidDetailed(const QString &name, const Config &cfg) {
  if (name.isEmpty()) return {.valid = false, .message = "Name is empty"};
  if (std::cmp_less(name.length(), cfg.kMinLenOfName)) return {.valid = false, .message = "Name too short"};
  if (std::cmp_greater(name.length(), cfg.kMaxLenOfName)) return {.valid = false, .message = "Name too long"};
  CharConfig config_for_name{
    .lettersAllowed = true,
    .numbersAllowed = true,
    .spaceAllowed = true,
    .specialCharactersAllowed = "-'"
  };

  for (QChar c : name) {
    if (!DataInputService::details::isValidChar(c, config_for_name)) {
      return {.valid = false, .message = "Name contains invalid character"};
    }
  }

  return {.valid = true, .message = ""};
}

ValidationResult emailValidDetailed(const QString &email, const Config &cfg) {
  if (email.isEmpty()) return {.valid = false, .message = "Email is empty"};

  const int at_pos = static_cast<int>(email.indexOf('@'));
  if (at_pos == -1) return {.valid = false, .message = "Email does not contain @"};

  const auto local = email.left(at_pos);
  const auto domain = email.mid(at_pos + 1);

  if (auto res = DataInputService::details::checkLocalPart(local, cfg); !res.valid) return res;
  if (auto res = DataInputService::details::checkDomainPart(domain, cfg); !res.valid) return res;

  return {.valid = true, .message = "Email is valid"};
}

ValidationResult passwordValidDetailed(const QString &password, const Config &cfg) {
  if (password.isEmpty()) return {.valid = false, .message = "Password is empty"};
  if (password.size() < cfg.kMinPasswordLength) return {.valid = false, .message = "Password is too short"};
  if (password.size() > cfg.kMaxPasswordLength) return {.valid = false, .message = "Password is too long"};
  CharConfig config{
    .lettersAllowed = true,
    .numbersAllowed = true,
    .spaceAllowed = false,
    .specialCharactersAllowed = QStringLiteral("!$_+@#%&*-")
  };

  for (const QChar &c : password) {
    if(!DataInputService::details::isValidChar(c, config)) {
      return {.valid = false, .message = "Password contains invalid character"};
    }
  }

  return {.valid = true, .message = "Password is valid"};
}

ValidationResult tagValidDetailed(const QString &tag, const Config &cfg) {
  if (tag.isEmpty()) return {.valid = false, .message = "Tag is empty"};
  if (std::cmp_less(tag.size(), cfg.kMinTagLength)) return {.valid = false, .message = "Tag too short"};
  if (std::cmp_greater(tag.size(), cfg.kMaxTagLength)) return {.valid = false, .message = "Tag too long"};
  if (!tag.front().isLetterOrNumber()) {
    return {.valid = false, .message = "First character must be letter or number"};
  }

  CharConfig config{
    .lettersAllowed = true,
    .numbersAllowed = true,
    .spaceAllowed = false,
    .specialCharactersAllowed = "_.-"
  };

  QChar prev_char = '\n';
  for (QChar c : tag) {
    if(c == '_' && prev_char == '_') {
        return {.valid = false, .message = "Tag can't contains two '_' in a row"};
    } else if(!DataInputService::details::isValidChar(c, config)) {
      return {.valid = false, .message = "Tag contains invalid character"};
    }
    prev_char = c;
  }

  return {.valid = true, .message = "Tag is valid"};
}

ValidationResult validateRegistrationUserInput(const SignUpRequest &input, const Config &cfg) {
  if (auto r = nameValidDetailed(input.name, cfg); !r.valid) return r;
  if (auto r = emailValidDetailed(input.email, cfg); !r.valid) return r;
  if (auto r = passwordValidDetailed(input.password, cfg); !r.valid) return r;
  if (auto r = tagValidDetailed(input.tag, cfg); !r.valid) return r;
  return {.valid = true, .message = "All fields valid"};
}

ValidationResult validateLoginUserInput(const LogInRequest &input, const Config &cfg) {
  if (auto r = emailValidDetailed(input.email, cfg); !r.valid) return r;
  if (auto r = passwordValidDetailed(input.password, cfg); !r.valid) return r;
  return {.valid = true, .message = "All fields valid"};
}

}  // namespace DataInputService

namespace DataInputService::details {

ValidationResult checkLocalPart(const QString &local, const Config &cfg) {
  if (local.isEmpty()) {
    return {.valid = false, .message = "Local part is empty"};
  }
  if (local.size() < cfg.kMinEmailLocalPartLength) {
    return {.valid = false, .message = "Local part too short"};
  }
  if (local.size() > cfg.kMaxEmailLocalPartLength) {
    return {.valid = false, .message = "Local part too long"};
  }

  CharConfig config;
  config.lettersAllowed = true;
  config.numbersAllowed = true;
  config.specialCharactersAllowed = "._";

  QChar prev_char = '\n';
  for (QChar el : local) {
    if((el == '.' || el == '_') && prev_char == el) {
      return {.valid = false, .message = "Local part contains consecutive invalid characters"};
    }
    if(!isValidChar(el, config)) {
      return {.valid = false, .message = "Local part contains invalid character"};
    }

    prev_char = el;
  }

  return {.valid = true, .message = "Local part is valid"};
}

ValidationResult checkDomainPart(const QString &domain, const Config &cfg) {
  if (domain.isEmpty()) return {.valid = false, .message = "Domain is empty"};
  if (!domainIsAllowedList(domain, cfg)) return {.valid = false, .message = "Invalid domain"};
  return {.valid = true, .message = "Domain part is good"};
}

bool isValidChar(QChar el, const CharConfig& config) {
    if(el.isLetter()) return config.lettersAllowed;
    if(el.isNumber()) return config.numbersAllowed;
    if(el.isSpace()) return config.spaceAllowed;
    return config.specialCharactersAllowed.contains(el);
}

}  // namespace DataInputService::details
