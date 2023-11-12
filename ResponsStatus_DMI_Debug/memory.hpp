#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// Target module representing a simple memory

class Memory : sc_module
{
public:
    enum
    {
        SIZE = 256
    };

    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<Memory> socket;
    int mem[SIZE];

    const sc_time LATENCY;

    SC_CTOR(Memory)
        : socket("socket"), LATENCY(10, SC_NS)
    {
        // Register callbacks for incoming interface method calls
        socket.register_b_transport(this, &Memory::b_transport);
        socket.register_get_direct_mem_ptr(this, &Memory::get_direct_mem_ptr);
        socket.register_transport_dbg(this, &Memory::transport_dbg);

        // Initialize memory with random data
        for (int i = 0; i < SIZE; i++)
            mem[i] = 0xAA000000 | (rand() % 256);

        SC_THREAD(invalidation_process);
    }

private:
    /**
     * @brief TLM-2 blocking transport method
     *
     * @param trans
     * @param delay
     */
    virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 adr = trans.get_address() / 4;
        unsigned char *ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();
        unsigned char *byt = trans.get_byte_enable_ptr();
        unsigned int wid = trans.get_streaming_width();

        // Obliged to check address range and check for unsupported features,
        //   i.e. byte enables, streaming, and bursts
        // Can ignore extensions

        // *********************************************
        // Generate the appropriate error response
        // *********************************************

        /**
         * If the transaction fails, the target can choose between a predefined set of error responses
         */
        if (adr >= sc_dt::uint64(SIZE))
        {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            std::cout << "Error: TLM_ADDRESS_ERROR_RESPONSE" << std::endl;
            return;
        }

        /**
         * The address error response should be used to indicate that the address is out-of-range or
         * that the transaction failed because of the value of the address given in the transaction.
         */
        if (byt != 0)
        {
            trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            std::cout << "Error: TLM_BYTE_ENABLE_ERROR_RESPONSE" << std::endl;
            return;
        }

        /**
         * The byte enable error response should be used to indicate either that the value of the byte enables in the transaction object
         * caused an error at the target, or that the target does not support byte enables at all, as is the case in this example.
         */
        if (len > 4 || wid < len)
        {
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
            std::cout << "Error: TLM_BURST_ERROR_RESPONSE" << std::endl;
            return;
        }

        // Obliged to implement read and write commands
        if (cmd == tlm::TLM_READ_COMMAND)
            memcpy(ptr, &mem[adr], len);
        else if (cmd == tlm::TLM_WRITE_COMMAND)
            memcpy(&mem[adr], ptr, len);

        // Illustrates that b_transport may block
        wait(delay);

        // Reset timing annotation after waiting
        delay = SC_ZERO_TIME;

        // *********************************************
        // Set DMI hint to indicated that DMI is supported
        // *********************************************

        /**
         * The target may signal to the initiator that it is able to support
         * the direct memory interface using the DMI hint attribute of the generic payload.
         * This can provide a simulation speedup for the initiator, because there is no point in the initiator
         * making repeated calls to get_direct_mem_ptr if it can be told in advance that such calls are going to fail.
         * Hence the b_transport method in our example makes the following call to set the DMI hint
         */
        trans.set_dmi_allowed(true);

        // Obliged to set response status to indicate successful completion
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    /**
     * @brief TLM-2 forward DMI method
     * Get the direct mem ptr object. It is called by the initiator along the forward path and is implemented by the target,
     * a memory in our example. The memory uses the simple_target_socket, and as for b_transport,
     * the target must register the implementation of the method with the socket.
     * Otherwise, the simple socket would supply a default implementation that takes no action.
     *
     * @param trans
     * @param dmi_data
     * @return true
     * @return false
     */
    virtual bool get_direct_mem_ptr(tlm::tlm_generic_payload &trans,
                                    tlm::tlm_dmi &dmi_data)
    {
        /**
         * The target (in this example is Memory) must decide whether or not it can grant the kind of access being requested,
         * and may even grant a higher level of access than requested.
         * In this example, the target grants read/write access whatever the mode of the request.
         * Ultimately the DMI transaction type is a template parameter, so applications can substitute their own modes of access where required.
         * Of course, using a non-standard DMI transaction type will limit interoperability,
         * just as substituting a non-standard type in place of the generic payload would limit interoperability when using the transport interface.
         */

        // The target must now populate the DMI data object to describe the details of the access being given.

        // Permit read and write access
        dmi_data.allow_read_write();

        // Set other details of DMI region
        dmi_data.set_dmi_ptr(reinterpret_cast<unsigned char *>(&mem[0]));
        dmi_data.set_start_address(0);
        dmi_data.set_end_address(SIZE * 4 - 1);
        dmi_data.set_read_latency(LATENCY);
        dmi_data.set_write_latency(LATENCY);

        return true;
    }

    void invalidation_process()
    {
        // Invalidate DMI pointers periodically
        for (int i = 0; i < 4; i++)
        {
            wait(LATENCY * 8);
            socket->invalidate_direct_mem_ptr(0, SIZE - 1);
        }
    }

    /**
     * @brief TLM-2 debug transport method
     * The purpose of the debug transport interface is to give an initiator the ability to read or write memory
     * in the target without causing any side-effects and without simulation time passing.
     * There are some similarities between DMI and debug, but the intent is very different.
     * DMI is intended to speed-up simulation during normal transactions, whereas the debug transport interface is exclusively intended for debug.
     * There is only one debug transport interface, and that uses the forward path from initiator to target.
     * The target must implement the transport_dbg method, and in the case of the simple target socket,
     * must register the method with the socket. Otherwise, as for the direct memory interface,
     * the simple socket would supply a default implementation that takes no action.
     *
     * @param trans
     * @return unsigned int
     */
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload &trans)
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 adr = trans.get_address() / 4;
        unsigned char *ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();

        // Calculate the number of bytes to be actually copied
        unsigned int num_bytes = (len < (SIZE - adr) * 4) ? len : (SIZE - adr) * 4;

        if (cmd == tlm::TLM_READ_COMMAND)
            memcpy(ptr, &mem[adr], num_bytes);
        else if (cmd == tlm::TLM_WRITE_COMMAND)
            memcpy(&mem[adr], ptr, num_bytes);

        return num_bytes;
    }
};
