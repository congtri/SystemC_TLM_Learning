#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

class Initiator : sc_module
{
private:
    /* data */
public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_initiator_socket<Initiator> socket;
    sc_time delay = sc_time(10, SC_NS);

    // This is the class constructor.
    SC_HAS_PROCESS(Initiator);

    Initiator(sc_core::sc_module_name name) : socket("socket") // Construct and name socket
    {
        // register thread process
        SC_THREAD(initiator_thread_process);
    }

    void initiator_thread_process()
    {
        // Realize the delay annotated onto the transport call
        wait(delay);
    }

    void sendCsvPath(string csv_file_path)
    {
        tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;
        sc_time delay = sc_time(10, SC_NS);
        int addr_cmd = 0xAABB;

        // Initialize 8 out of the 10 attributes, byte_enable_length and extensions being unused
        trans->set_command(tlm::TLM_WRITE_COMMAND);                             // Set the command for the transaction. The cmd variable likely holds a value from the tlm::tlm_command enumeration, indicating the type of transaction (e.g., read or write).
        trans->set_address(addr_cmd);                                           // Set the address for the transaction. This line is configuring the address where the transaction will read from or write to.
        trans->set_data_ptr(reinterpret_cast<unsigned char *>(&csv_file_path)); // Set the data pointer for the transaction, converting to unsigned char*. This pointer references the data to be transferred.
        trans->set_data_length(csv_file_path.length());                         // Set the length of the data associated with the transaction to 4 bytes.                                        // Set the streaming width for burst transfers (4 bytes, indicating no streaming).
        trans->set_byte_enable_ptr(0);                                          // Set the byte-enable pointer for the transaction. 0 indicates that byte enables are not used.
        trans->set_dmi_allowed(false);                                          // Set whether DMI (Direct Memory Interface) is allowed for this transaction. DMI allows direct access to memory without regular transaction processing.
        trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);               // Set the response status of the transaction to indicate an incomplete response. Actual response status may be updated based on the outcome of the transaction.

        socket->b_transport(*trans, delay); // Blocking transport call

        // Initiator obliged to check response status and delay
        if (trans->is_response_error())
        {
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }
    }

private:
    string database_file_ = "";
};
