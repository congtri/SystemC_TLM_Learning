#include "systemc"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "CsvReader.hpp"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

class ReceiverModel : sc_module
{
private:
    /* data */
    CsvReader table_data;

public:
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<ReceiverModel> socket;

    ReceiverModel(sc_core::sc_module_name name) : socket("socket")
    {
        // Register callback for incoming b_transport interface method call
        socket.register_b_transport(this, &ReceiverModel::b_transport);

        table_data.clearData();
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

        // Obliged to implement read and write commands
        if (cmd == tlm::TLM_WRITE_COMMAND)
        {
            // Assuming the buffer contains a null-terminated string, convert it to std::string
            std::string csv_path(reinterpret_cast<const char *>(ptr));
            if (table_data.readCsv(csv_path))
            {
                // Example usage:
                std::cout << "Table Header:\n";
                const auto &header = table_data.getTableHeader();
                for (const auto &column : header)
                {
                    std::cout << column << " ";
                }
                std::cout << "\n";

                // Get header with column indices
                auto headerWithIndices = table_data.getHeaderWithIndices();
                std::cout << "Header with Indices:\n";
                for (const auto &pair : headerWithIndices)
                {
                    std::cout << pair.first << " at index " << pair.second << "\n";
                }

                std::cout << "Total Rows: " << table_data.getTotalRows() << "\n";
                std::cout << "Total Columns: " << table_data.getTotalColumns() << "\n";

                // Print cell values in the first row
                for (size_t col = 0; col < table_data.getTotalColumns(); ++col)
                {
                    std::cout << "Cell(" << 0 << "," << col << "): " << table_data.getCellValue(0, col) << "\n";
                }

                // Obliged to set response status to indicate successful completion
                trans.set_response_status(tlm::TLM_OK_RESPONSE);
            }
        }
    }
};
