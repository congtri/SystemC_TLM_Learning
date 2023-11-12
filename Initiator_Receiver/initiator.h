#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

/// Initiator module generating generic payload transactions

class Initiator : sc_module
{
public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_initiator_socket<Initiator> socket;

    // This is the class constructor.
    SC_HAS_PROCESS(Initiator);
    Initiator(sc_core::sc_module_name name) : socket("socket") // Construct and name socket
    {
        // register thread process
        SC_THREAD(thread_process);
    }

private:
    // Internal data buffer used by initiator with generic payload
    int data;

    void thread_process()
    {
        // TLM-2 generic payload transaction, reused across calls to b_transport
        tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;
        sc_time delay = sc_time(10, SC_NS);

        // Generate a random sequence of reads and writes
        for (int i = 32; i < 96; i += 4)
        {

            tlm::tlm_command cmd = static_cast<tlm::tlm_command>(rand() % 2);
            if (cmd == tlm::TLM_WRITE_COMMAND)
                data = 0xFF000000 | i;

            // Initialize 8 out of the 10 attributes, byte_enable_length and extensions being unused
            trans->set_command(cmd);                                       // Set the command for the transaction. The cmd variable likely holds a value from the tlm::tlm_command enumeration, indicating the type of transaction (e.g., read or write).
            trans->set_address(i);                                         // Set the address for the transaction. This line is configuring the address where the transaction will read from or write to.
            trans->set_data_ptr(reinterpret_cast<unsigned char *>(&data)); // Set the data pointer for the transaction, converting to unsigned char*. This pointer references the data to be transferred.
            trans->set_data_length(4);                                     // Set the length of the data associated with the transaction to 4 bytes.
            trans->set_streaming_width(4);                                 // Set the streaming width for burst transfers (4 bytes, indicating no streaming).
            trans->set_byte_enable_ptr(0);                                 // Set the byte-enable pointer for the transaction. 0 indicates that byte enables are not used.
            trans->set_dmi_allowed(false);                                 // Set whether DMI (Direct Memory Interface) is allowed for this transaction. DMI allows direct access to memory without regular transaction processing.
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);      // Set the response status of the transaction to indicate an incomplete response. Actual response status may be updated based on the outcome of the transaction.

            socket->b_transport(*trans, delay); // Blocking transport call

            // Initiator obliged to check response status and delay
            if (trans->is_response_error())
            {
                SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
            }

            cout << "trans = { " << (cmd ? 'W' : 'R') << ", " << hex << i
                 << " } , data = " << hex << data << " at time " << sc_time_stamp()
                 << " delay = " << delay << endl;

            // Realize the delay annotated onto the transport call
            wait(delay);
        }
    }
};
