#include "DataInputService.h"

#include <QChar>
#include <QLocale>
#include <QMutex>
#include <QRegularExpression>
#include <QString>
#include <algorithm>

namespace {

bool isAsciiLetterOrNumber(QChar ch) { return ch.isLetterOrNumber(); }

bool isControlOrSpace(QChar ch) {
  const ushort code = ch.unicode();
  return ch.isSpace() || (code < 0x20) || (code == 0x7F);
}

int utf8Length(const QString &str) {
  int len = 0;
  for (const QChar &c : str) {
    if ((c.unicode() & 0xC0) != 0x80)
      ++len;
  }
  return len;
}

} // namespace

namespace DataInputService {

inline void loadDomains(const Config &cfg) {
  if (!cfg.kDomains.isEmpty()) {
    return;
  }

  QFile file(cfg.kConfigDomainsPath);
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (!line.isEmpty())
        cfg.kDomains.append(line);
    }
  } else {
    qWarning() << "Cannot open domains file:" << cfg.kConfigDomainsPath;
  }
}

static bool domainIsAllowedList(const QString &domain, const Config &cfg) {
  loadDomains(cfg);
  const QString normalized = domain.toLower();
  return cfg.kDomains.contains(normalized);
}

ValidationResult nameValidDetailed(const QString &name, const Config &cfg) {
  if (name.isEmpty())
    return {.valid = false, .message = "Name is empty"};

  // Count only letters and digits for length checks (ignore spaces, hyphens,
  // apostrophes)
  int meaningful_len = 0;
  for (const QChar &c : name) {
    if (c.category() == QChar::Other_Control)
      return {.valid = false, .message = "Name contains invalid character"};

    // treat letters and numbers as meaningful
    if (c.isLetter() || c.isNumber()) {
      ++meaningful_len;
    }
  }

  if (std::cmp_less(meaningful_len, cfg.kMinLenOfName))
    return {.valid = false, .message = "Name too short"};
  if (std::cmp_greater(meaningful_len, cfg.kMaxLenOfName))
    return {.valid = false, .message = "Name too long"};

  for (const QChar &c : name) {
    if (c.category() == QChar::Other_Control)
      return {.valid = false, .message = "Name contains invalid character"};

    const ushort u = c.unicode();
    if (u < 128) {
      if (!c.isLetterOrNumber() && !c.isSpace() && c != QLatin1Char('-') &&
          c != QLatin1Char('\'')) {
        return {.valid = false, .message = "Name contains invalid character"};
      }
    } else {
      if (!c.isLetter() && !c.isSpace()) {
        return {.valid = false, .message = "Name contains invalid character"};
      }
    }
  }

  return {.valid = true, .message = ""};
}

ValidationResult emailValidDetailed(const QString &email, const Config &cfg) {
  if (email.isEmpty())
    return {.valid = false, .message = "Email is empty"};

  const int at_pos = email.indexOf('@');
  if (at_pos == -1)
    return {.valid = false, .message = "Email does not contain @"};

  const QString local = email.left(at_pos);
  const QString domain = email.mid(at_pos + 1);

  if(auto res = DataInputService::details::checkLocalPart(local, cfg); !res.valid) return res;
  if(auto res = DataInputService::details::checkDomainPart(domain, cfg); !res.valid) return res;

  return {.valid = true, .message = "Email is valid"};
}

ValidationResult passwordValidDetailed(const QString &password,
                                       const Config &cfg) {
  if (password.isEmpty())
    return {.valid = false, .message = "Password is empty"};
  if (password.size() < cfg.kMinPasswordLength)
    return {.valid = false, .message = "Password is too short"};
  if (password.size() > cfg.kMaxPasswordLength)
    return {.valid = false, .message = "Password is too long"};

  static const QString allowedSymbols = QStringLiteral("!$_+@#%&*-");
  for (const QChar &c : password) {
    if (c.unicode() >= 0x80)
      continue;
    if (isControlOrSpace(c))
      return {.valid = false,
              .message = "Password contains space or control character"};
    if (c.isLetterOrNumber())
      continue;
    if (allowedSymbols.contains(c))
      continue;

    return {.valid = false, .message = "Password contains invalid character"};
  }
  return {.valid = true, .message = "Password is valid"};
}

ValidationResult tagValidDetailed(const QString &tag, const Config &cfg) {
  if (tag.isEmpty())
    return {.valid = false, .message = "Tag is empty"};
  if (std::cmp_less(tag.size(), cfg.kMinTagLength))
    return {.valid = false, .message = "Tag too short"};
  if (std::cmp_greater(tag.size(), cfg.kMaxTagLength))
    return {.valid = false, .message = "Tag too long"};

  const QChar first = tag.front();
  if (first.unicode() < 0x80 && !first.isLetterOrNumber())
    return {.valid = false,
            .message = "First character must be letter or number"};

  bool prevWasUnderscore = false;
  for (const QChar &c : tag) {
    if (c.unicode() >= 0x80) {
      prevWasUnderscore = false;
      continue;
    }
    if (c.isLetterOrNumber()) {
      prevWasUnderscore = false;
      continue;
    }
    if (c == '_') {
      if (prevWasUnderscore)
        return {.valid = false,
                .message = "Tag contains consecutive underscores"};
      prevWasUnderscore = true;
      continue;
    }
    if (c == '-' || c == '.') {
      prevWasUnderscore = false;
      continue;
    }

    return {.valid = false, .message = "Tag contains invalid character"};
  }
  return {.valid = true, .message = "Tag is valid"};
}

ValidationResult validateRegistrationUserInput(const SignUpRequest &input,
                                               const Config &cfg) {
  if (auto r = nameValidDetailed(input.name, cfg); !r.valid) {
    return r;
  }

  if (auto r = emailValidDetailed(input.email, cfg); !r.valid) {
    return r;
  }

  if (auto r = passwordValidDetailed(input.password, cfg); !r.valid) {
    return r;
  }

  if (auto r = tagValidDetailed(input.tag, cfg); !r.valid) {
    return r;
  }

  return {.valid = true, .message = "All fields valid"};
}

ValidationResult validateLoginUserInput(const LogInRequest &input,
                                        const Config &cfg) {
  if (auto r = emailValidDetailed(input.email, cfg); !r.valid)
    return r;

  if (auto r = passwordValidDetailed(input.password, cfg); !r.valid)
    return r;

  return {.valid = true, .message = "All fields valid"};
}

}  // namespace DataInputService

namespace DataInputService::details {

ValidationResult checkLocalPart(const QString& local, const Config &cfg) {
  if (local.isEmpty())
    return {false, "Local part is empty"};

  if (std::cmp_less(local.size(), cfg.kMinEmailLocalPartLength))
    return {.valid = false, .message = "Local part too short"};

  if (std::cmp_greater(local.size(), cfg.kMaxEmailLocalPartLength))
    return {.valid = false, .message = "Local part too long"};

  const bool is_quoted =
      (local.size() >= 2 && local.front() == '"' && local.back() == '"');

  if (is_quoted)  return {.valid = true,
            .message = "Local part is good"};

  if (local.startsWith('.') || local.endsWith('.'))
    return {false, "Local part starts/ends with dot"};

  for (int i = 0; i < local.size(); ++i) {
    const QChar c = local[i];
    if (c.unicode() < 0x20)
      return {.valid = false,
              .message = "Local part contains control characters"};

    if (c == '.' && i + 1 < local.size() && local[i + 1] == '.') {
      return {.valid = false, .message = "Local part has consecutive dots"};
    }

    static const QString kAllowedSymbols =
        QStringLiteral("!#$%&'*+-/=?^_`{|}~");

    if (!c.isLetterOrNumber() && !kAllowedSymbols.contains(c) && c != '+') {
      return {.valid = false,
            .message = "Local part contains invalid character"};
    }
  }

  return {.valid = true,
          .message = "Local part is valid"};
}

ValidationResult checkDomainPart(const QString& domain, const Config &cfg) {
  if (domain.isEmpty())
    return {.valid = false, .message = "Domain is empty"};

  if (domain.size() > 255)
    return {false, "Domain too long"};

  if (!domainIsAllowedList(domain, cfg))
    return {.valid = false, .message = "Invalid domain"};

  // if (domain.startsWith('[') && domain.endsWith(']')) {
  //   const QString inside = domain.mid(1, domain.size() - 2);
  //   if (inside.isEmpty())
  //     return {false, "Empty IP literal"};
  //   for (const QChar &c : inside) {
  //     if (!c.isDigit() && c != '.' && c != ':')
  //       return {false, "IP literal contains invalid character"};
  //   }
  //   return {true, "Email is valid"};
  // }


  // const QStringList labels = domain.split('.');
  // for (const QString &label : labels) {
  //   if (label.isEmpty())
  //     return {false, "Domain contains empty label"};
  //   if (label.startsWith('-') || label.endsWith('-'))
  //     return {false, "Label starts or ends with '-' character"};
  // }

  return {.valid = true,
          .message = "Domain part is good"};
}

} // namespace DataInputService::details
