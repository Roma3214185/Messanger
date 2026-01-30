#include "authservice/PasswordService.h"

#include <sodium.h>
#include <stdexcept>

namespace PasswordService {

std::string hash(const std::string &password) {
  char hash[crypto_pwhash_STRBYTES];
  if (crypto_pwhash_str(hash, password.c_str(), password.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE,
                        crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
    throw std::runtime_error("out of memory");
  }
  return std::string(hash);
}

bool verify(const std::string &raw_password, const std::string &hashed_password) {
  return crypto_pwhash_str_verify(hashed_password.c_str(), raw_password.c_str(), raw_password.size()) == 0;
}

}  // namespace PasswordService
