#ifndef PASSWORDSERVICE_H
#define PASSWORDSERVICE_H

#include <sodium.h>
#include <string>
#include <stdexcept>

namespace PasswordService {

inline std::string hash(const std::string& password) {
  char hash[crypto_pwhash_STRBYTES];
  if (crypto_pwhash_str(
          hash,
          password.c_str(),
          password.size(),
          crypto_pwhash_OPSLIMIT_INTERACTIVE,
          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
    throw std::runtime_error("out of memory");
  }
  return std::string(hash);
}

inline bool verify(const std::string& raw_password, const std::string& hashed_password) {
  return crypto_pwhash_str_verify(hashed_password.c_str(), raw_password.c_str(), raw_password.size()) == 0;
}

} // namespace PasswordService

#endif // PASSWORDSERVICE_H
