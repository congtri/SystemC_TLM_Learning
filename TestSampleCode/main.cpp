#include <systemc>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <vector>
#include <stdint.h>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <array>

constexpr int BUFFER_SIZE = 10;

void copy_data(std::vector<uint16_t>& dest, const uint8_t* src, uint8_t len) {
    dest.insert(dest.end(), reinterpret_cast<const uint16_t*>(src), reinterpret_cast<const uint16_t*>(src + len));
}

class Initiator : public sc_module
{
public:
    tlm_utils::simple_initiator_socket<Initiator> socket;

    SC_HAS_PROCESS(Initiator);
    Initiator(sc_core::sc_module_name name) : socket("socket")
    {
        SC_THREAD(thread_process);
    }

private:
    void thread_process()
    {
        // Generate dummy data
        std::array<uint16_t, BUFFER_SIZE> data_buffer;
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            data_buffer[i] = 0xAA00 + i;
        }
        
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(10, SC_NS);

        trans.set_command(tlm::tlm_command::TLM_WRITE_COMMAND);
        trans.set_address(rand());
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(data_buffer.data()));
        trans.set_data_length(sizeof(data_buffer));
        trans.set_streaming_width(1);
        trans.set_byte_enable_ptr(0);
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

        socket->b_transport(trans, delay);

        if (trans.is_response_error())
        {
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }
    }
};

class Memory : public sc_module
{
public:
    tlm_utils::simple_target_socket<Memory> socket;
    std::vector<uint16_t> rx_buffer;

    Memory(sc_core::sc_module_name name) : socket("socket")
    {
        socket.register_b_transport(this, &Memory::b_transport);
        rx_buffer.clear();
    }

    virtual void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        unsigned char *ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();

        if (cmd == tlm::TLM_WRITE_COMMAND)
        {
            // Use emplace_back for adding elements
            copy_data(rx_buffer, ptr, len);
        }

        // Process elements using range-based for loop
        for (const auto& element : rx_buffer)
        {
            // Process the element (e.g., print it)
            std::cout << "Processing element: " << std::hex << element << std::endl;
        }

        // Clear the buffer after processing the data
        rx_buffer.clear();

        // Set response status to indicate successful completion
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }
};


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