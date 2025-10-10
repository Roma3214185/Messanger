#include <QCoreApplication>
#include <QDebug>
// #include <amqpcpp.h>
// #include <amqpcpp/libevent.h>
#include <event2/event.h>
#include <crow.h>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // struct event_base *evbase = event_base_new();
    // AMQP::LibEventHandler handler(evbase);
    // AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://guest:guest@localhost/"));
    // AMQP::TcpChannel channel(&connection);

    // channel.declareExchange("chat_exchange", AMQP::fanout);
    // channel.declareQueue("notify_queue").onSuccess([&channel](const std::string &name, int, int) {
    //     channel.bindQueue("chat_exchange", name, "");
    //     channel.consume(name).onReceived([](const AMQP::Message &msg, uint64_t, bool) {
    //         QString body = QString::fromStdString(std::string(msg.body(), msg.bodySize()));
    //         qDebug() << "📢 New notification:" << body;
    //     });
    // });

    // event_base_dispatch(evbase);
    // return a.exec();


    crow::SimpleApp app;

    CROW_ROUTE(app, "/notify").methods("POST"_method)([](const crow::request& req){
        std::cout << "📢 New notification: " << req.body << std::endl;
        return crow::response(200, "OK");
    });

    app.port(8080).multithreaded().run();
}
