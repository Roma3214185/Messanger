#include "usecases/DefaultUseCaseRepository.h"

DefaultUseCaseRepository::DefaultUseCaseRepository(std::unique_ptr<IChatUseCase> chat,
                                                   std::unique_ptr<IMessageUseCase> message,
                                                   std::unique_ptr<IUserUseCase> user,
                                                   std::unique_ptr<ISessionUseCase> session,
                                                   std::unique_ptr<ISocketUseCase> socket)
    : chat_(std::move(chat)),
      message_(std::move(message)),
      user_(std::move(user)),
      session_(std::move(session)),
      socket_(std::move(socket)) {}

IChatUseCase* DefaultUseCaseRepository::chat() { return chat_.get(); }
IMessageUseCase* DefaultUseCaseRepository::message() { return message_.get(); }
IUserUseCase* DefaultUseCaseRepository::user() { return user_.get(); }
ISessionUseCase* DefaultUseCaseRepository::session() { return session_.get(); }
ISocketUseCase* DefaultUseCaseRepository::socket() { return socket_.get(); }
