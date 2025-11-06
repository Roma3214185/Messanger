#include <QCoreApplication>

#include "Debug_profiling.h"
#include "GenericRepository.h"
#include "managers/MessageManager.h"
#include "managers/notificationmanager.h"
#include "server.h"
#include "GenericRepository/Batcher.h"
#include "RabbitMQClient/rabbitmqclient.h"

const int MESSAGE_PORT = 8082;
constexpr int kRabitMQPort = 5672;

int main(int argc, char *argv[]) {
    init_logger("MessageService");
    QCoreApplication a(argc, argv);
    SQLiteDatabase bd;

    GenericRepository genetic_rep(bd);

    SaverBatcher<Message> message_saver_batcher(genetic_rep);
    SaverBatcher<MessageStatus> message_status_saver_batcher(genetic_rep);
    DeleterBatcher<Message> message_delete_batcher(genetic_rep);
    DeleterBatcher<MessageStatus> message_status_delete_batcher(genetic_rep);

    Batcher<Message> message_batcher(message_saver_batcher, message_delete_batcher);
    Batcher<MessageStatus> message_status_batcher(message_status_saver_batcher, message_status_delete_batcher);

    MessageManager manager(&genetic_rep, &message_batcher, &message_status_batcher);
    RabbitMQClient* mq = nullptr;

    try {
      mq = new RabbitMQClient("localhost", kRabitMQPort, "guest", "guest");
    } catch (const AmqpClient::AmqpLibraryException& e) {
      LOG_ERROR("Cannot connect to RabbitMQ: {}", e.what());
    }

    Server server(MESSAGE_PORT, &manager, mq);
    server.run();

    return a.exec();
}


