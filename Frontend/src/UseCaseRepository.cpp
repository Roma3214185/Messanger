#include "usecases/UseCaseRepository.h"

UseCaseRepository::UseCaseRepository(std::unique_ptr<ChatUseCase> chat,
                                    std::unique_ptr<MessageUseCase> message,
                                    std::unique_ptr<UserUseCase> user,
                                    std::unique_ptr<SessionUseCase> session,
                                    std::unique_ptr<SocketUseCase> socket)
    : chat_(std::move(chat)),
      message_(std::move(message)),
      user_(std::move(user)),
      session_(std::move(session)),
      socket_(std::move(socket)) {}

ChatUseCase* UseCaseRepository::chat() { return chat_.get(); }
MessageUseCase* UseCaseRepository::message() { return message_.get(); }
UserUseCase* UseCaseRepository::user() { return user_.get(); }
SessionUseCase* UseCaseRepository::session() { return session_.get(); }
SocketUseCase* UseCaseRepository::socket() { return socket_.get(); }
