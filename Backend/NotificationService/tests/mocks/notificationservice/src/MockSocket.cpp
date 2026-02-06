#include "mocks/notificationservice/MockSocket.h"

void MockSocket::send_text(const std::string &text) {
    ++send_text_calls;
    last_sended_text = text;
}
