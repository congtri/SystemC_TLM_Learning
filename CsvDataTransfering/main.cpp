#include "Initiator.hpp"
#include "ReceiverModel.hpp"

int main(void)
{
    Initiator *initiator;
    ReceiverModel *receiver;

    // Instantiate components
    initiator = new Initiator("initiator");
    receiver = new ReceiverModel("receiver");

    // One initiator is bound directly to one target with no intervening bus

    // Bind initiator socket to target socket
    initiator->socket.bind(receiver->socket);
    initiator->sendCsvPath("example.csv");

    sc_start();
}
