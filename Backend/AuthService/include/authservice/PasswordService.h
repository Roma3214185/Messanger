#ifndef PASSWORDSERVICE_H
#define PASSWORDSERVICE_H

#include <string>

namespace PasswordService {

std::string hash(const std::string &password);
bool verify(const std::string &raw_password, const std::string &hashed_password);

}  // namespace PasswordService

#endif  // PASSWORDSERVICE_H
