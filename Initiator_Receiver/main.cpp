#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "initiator.h"
#include "memory.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;


int sc_main(int argc, char *argv[])
{
    Initiator *initiator;
    Memory *memory;

    // Instantiate components
    initiator = new Initiator("initiator");
    memory = new Memory("memory");

    // One initiator is bound directly to one target with no intervening bus

    // Bind initiator socket to target socket
    initiator->socket.bind(memory->socket);

    sc_start();
    return 0;
}