#include "DataInputService.h"
#include <QRegularExpression>
#include <QString>
#include <QChar>
#include <QLocale>
#include <QMutex>

namespace DataInputService {

static bool isAsciiLetterOrNumber(QChar ch) {
  return ch.isLetterOrNumber();
}

static bool isControlOrSpace(QChar ch) {
  ushort code = ch.unicode();
  return ch.isSpace() || (code < 0x20) || (code == 0x7F);
}

static int utf8Length(const QString &str) {
  int len = 0;
  for (const QChar &c : str) {
    if ((c.unicode() & 0xC0) != 0x80)
      ++len;
  }
  return len;
}

static QSet<QString> g_validDomains;
static QMutex g_domainMutex;
static bool g_domainsLoaded = false;

static void loadValidDomainsOnce(const QString &path)
{
  QMutexLocker locker(&g_domainMutex);
  if (g_domainsLoaded) return;

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    LOG_ERROR("Can't open file with valid domains: {}", path.toStdString());
    return;
  }

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString domain = in.readLine().trimmed();
    if (!domain.isEmpty())
      g_validDomains.insert(domain.toLower());
  }

  g_domainsLoaded = true;
  LOG_INFO("Loaded {} valid domains", g_validDomains.size());
}

static bool isValidDomain(const QString &domain, const Config &cfg)
{
  if (!g_domainsLoaded)
    loadValidDomainsOnce(cfg.kConfigDomainsRoad);

  const QString normalized = domain.toLower();
  return g_validDomains.contains(normalized);
}

ValidationResult nameValidDetailed(const QString &name, const Config &cfg) {
  if (name.isEmpty())
    return {false, "Name is empty"};

  const int len = utf8Length(name);
  if (len < cfg.kMinLenOfName)
    return {false, "Name too short"};
  if (len > cfg.kMaxLenOfName)
    return {false, "Name too long"};

  for (const QChar &c : name) {
    if (c.unicode() < 128) {
      if (!c.isLetterOrNumber() && c != QLatin1Char(' ')
          && c != QLatin1Char('-') && c != QLatin1Char('\'')) {
        return {false, "Name contains invalid character"};
      }
    }
  }
  return {true, ""};
}

ValidationResult emailValidDetailed(const QString &email, const Config &cfg) {
  if (email.isEmpty())
    return {false, "Email is empty"};

  const int atPos = email.indexOf('@');
  if (atPos == -1)
    return {false, "Email does not contain @"};

  const QString local = email.left(atPos);
  const QString domain = email.mid(atPos + 1);

  if(!isValidDomain(domain, cfg))
    return {false, "InvalidDomain"};
  if (local.isEmpty())
    return {false, "Local part is empty"};
  if (local.size() < cfg.kMinEmailLocalPartLength)
    return {false, "Local part too short"};
  if (local.size() > cfg.kMaxEmailLocalPartLength)
    return {false, "Local part too long"};

  const bool isQuoted = (local.size() >= 2 && local.front() == '"' && local.back() == '"');
  if (!isQuoted) {
    if (local.startsWith('.') || local.endsWith('.'))
      return {false, "Local part starts/ends with dot"};

    for (int i = 0; i < local.size(); ++i) {
      const QChar c = local[i];
      if (c.unicode() < 0x20)
        return {false, "Local part contains control characters"};

      if (c == '.') {
        if (i + 1 < local.size() && local[i + 1] == '.')
          return {false, "Local part has consecutive dots"};
        continue;
      }

      static const QString allowedSymbols = QStringLiteral("!#$%&'*+-/=?^_`{|}~");
      if (c.isLetterOrNumber() || allowedSymbols.contains(c) || c == '+')
        continue;

      return {false, "Local part contains invalid character"};
    }
  }

  if (domain.isEmpty())
    return {false, "Domain is empty"};

  if (domain.startsWith('[') && domain.endsWith(']')) {
    const QString inside = domain.mid(1, domain.size() - 2);
    if (inside.isEmpty())
      return {false, "Empty IP literal"};
    for (const QChar &c : inside) {
      if (!c.isDigit() && c != '.' && c != ':')
        return {false, "IP literal contains invalid character"};
    }
    return {true, "Email is valid"};
  }

  if (domain.size() > 255)
    return {false, "Domain too long"};

  const QStringList labels = domain.split('.');
  for (const QString &label : labels) {
    if (label.isEmpty())
      return {false, "Domain contains empty label"};
    if (label.startsWith('-') || label.endsWith('-'))
      return {false, "Label starts or ends with '-' character"};
  }

  return {true, "Email is valid"};
}

ValidationResult passwordValidDetailed(const QString &password, const Config &cfg) {
  if (password.isEmpty())
    return {false, "Password is empty"};
  if (password.size() < cfg.kMinPasswordLength)
    return {false, "Password is too short"};
  if (password.size() > cfg.kMaxPasswordLength)
    return {false, "Password is too long"};

  static const QString allowedSymbols = QStringLiteral("!$_+@#%&*-");
  for (const QChar &c : password) {
    if (c.unicode() >= 0x80)
      continue;
    if (isControlOrSpace(c))
      return {false, "Password contains space or control character"};
    if (c.isLetterOrNumber())
      continue;
    if (allowedSymbols.contains(c))
      continue;

    return {false, "Password contains invalid character"};
  }
  return {true, "Password is valid"};
}

ValidationResult tagValidDetailed(const QString &tag, const Config &cfg) {
  if (tag.isEmpty())
    return {false, "Tag is empty"};
  if (tag.size() < cfg.kMinTagLength)
    return {false, "Tag too short"};
  if (tag.size() > cfg.kMaxTagLength)
    return {false, "Tag too long"};

  const QChar first = tag.front();
  if (first.unicode() < 0x80 && !first.isLetterOrNumber())
    return {false, "First character must be letter or number"};

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
        return {false, "Tag contains consecutive underscores"};
      prevWasUnderscore = true;
      continue;
    }
    if (c == '-' || c == '.') {
      prevWasUnderscore = false;
      continue;
    }

    return {false, "Tag contains invalid character"};
  }
  return {true, "Tag is valid"};
}

ValidationResult validateRegistrationUserInput(const SignUpRequest& input, const Config& cfg) {
  ValidationResult r = nameValidDetailed(input.name, cfg);
  if (!r.valid) return r;

  r = emailValidDetailed(input.email, cfg);
  if (!r.valid) return r;

  r = passwordValidDetailed(input.password, cfg);
  if (!r.valid) return r;

  r = tagValidDetailed(input.tag, cfg);
  if (!r.valid) return r;

  return {true, "All fields valid"};
}

ValidationResult validateLoginUserInput(const LogInRequest& input, const Config& cfg) {
  ValidationResult r = emailValidDetailed(input.email, cfg);
  if (!r.valid) return r;

  r = passwordValidDetailed(input.password, cfg);
  if (!r.valid) return r;

  return {true, "All fields valid"};
}

} // namespace DataInputService
