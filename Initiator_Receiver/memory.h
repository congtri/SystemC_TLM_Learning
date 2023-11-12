#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

// Target module representing a simple memory
class Memory : sc_module
{
private:
    enum
    {
        SIZE = 256
    };

public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<Memory> socket;

    Memory(sc_core::sc_module_name name) : socket("socket")
    {
        // Register callback for incoming b_transport interface method call
        socket.register_b_transport(this, &Memory::b_transport);

        // Initialize memory with random data
        for (int i = 0; i < SIZE; i++)
            mem[i] = 0xAA000000 | (rand() % 256);
    }

    // TLM-2 blocking transport method
    virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        tlm::tlm_command cmd = trans.get_command();       // Get transaction commands such as TLM_READ_COMMAND or TLM_WRITE_COMMAND.
        sc_dt::uint64 adr = trans.get_address() / 4;      // Get get transaction address. The division by 4 suggests that the address might be byte-addressed, and this operation converts it to a word-addressed format.
        unsigned char *ptr = trans.get_data_ptr();        // This pointer can be used to access or manipulate the data being transferred in the transaction
        unsigned int len = trans.get_data_length();       // This variable indicates the number of bytes in the data buffer.
        unsigned char *byt = trans.get_byte_enable_ptr(); // Byte enables are used to specify which bytes in a data buffer are valid or should be modified during the transaction.
        unsigned int wid = trans.get_streaming_width();   // Streaming width is used in burst transfers to specify the number of bytes that can be transferred in a single burst

        // Obliged to check address range and check for unsupported features,
        //   i.e. byte enables, streaming, and bursts
        // Can ignore DMI hint and extensions
        // Using the SystemC report handler is an acceptable way of signalling an error

        if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
            SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");

        // Obliged to implement read and write commands
        if (cmd == tlm::TLM_READ_COMMAND)
        {
            // Read from memory
            memcpy(ptr, &mem[adr], len);
        }
        else if (cmd == tlm::TLM_WRITE_COMMAND)
        {
            // Store to memory
            memcpy(&mem[adr], ptr, len);
        }

        // Obliged to set response status to indicate successful completion
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    int mem[SIZE];
};