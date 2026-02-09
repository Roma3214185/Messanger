#include "usecases/UseCaseRepository.h"

UseCaseRepository::UseCaseRepository(std::unique_ptr<IChatUseCase> chat,
                                    std::unique_ptr<IMessageUseCase> message,
                                    std::unique_ptr<IUserUseCase> user,
                                    std::unique_ptr<ISessionUseCase> session,
                                    std::unique_ptr<ISocketUseCase> socket)
    : chat_(std::move(chat)),
      message_(std::move(message)),
      user_(std::move(user)),
      session_(std::move(session)),
      socket_(std::move(socket)) {}

IChatUseCase* UseCaseRepository::chat() { return chat_.get(); }
IMessageUseCase* UseCaseRepository::message() { return message_.get(); }
IUserUseCase* UseCaseRepository::user() { return user_.get(); }
ISessionUseCase* UseCaseRepository::session() { return session_.get(); }
ISocketUseCase* UseCaseRepository::socket() { return socket_.get(); }
