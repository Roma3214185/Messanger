#ifndef PROXYREPOSITORY_H
#define PROXYREPOSITORY_H

// #include "proxyclient.h"
// #include "interfaces/IConfigProvider.h"
// #include "ProdConfigProvider.h"

// class ProxyRepository {
//   public:
//     ProxyRepository(IConfigProvider* provider = &ProdConfigProvider::instance())
//         : provider_(provider)
//         , auth_proxy_(provider->ports().authService)
//         , chat_proxy_(provider->ports().chatService)
//         , message_proxy_(provider->ports().messageService)
//         , user_proxy_(provider->ports().userService)
//         , notification_proxy_(provider->ports().notificationService) {

//     }

//     ProxyClient& auth() { return auth_proxy_; }
//     ProxyClient& chat() { return chat_proxy_; }
//     ProxyClient& message() { return message_proxy_; }
//     ProxyClient& user() { return user_proxy_; }
//     ProxyClient& notification() { return notification_proxy_; }


//   private:
//     IConfigProvider* provider_;

//     ProxyClient auth_proxy_;
//     ProxyClient chat_proxy_;
//     ProxyClient message_proxy_;
//     ProxyClient user_proxy_;
//     ProxyClient notification_proxy_;
// };

#endif // PROXYREPOSITORY_H
