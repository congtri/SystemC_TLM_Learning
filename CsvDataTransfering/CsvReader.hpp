#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class CsvReader
{
private:
    std::vector<std::vector<std::string>> data_; // Matrix to store CSV data
    std::vector<std::string> tableHeader_;       // Header of the CSV

public:
    // Function to read CSV file
    bool readCsv(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open the file '" << filename << "'\n";
            return false;
        }

        std::string line;
        if (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string cell;

            while (std::getline(ss, cell, ','))
            {
                tableHeader_.push_back(cell);
            }
        }

        while (std::getline(file, line))
        {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string cell;

            while (std::getline(ss, cell, ','))
            {
                row.push_back(cell);
            }

            data_.push_back(row);
        }

        file.close();
        std::cout << "Loading data completed: " << filename << std::endl;
        return true;
    }

    // Function to get the value of a cell at a specific row and column
    std::string getCellValue(size_t row, size_t col) const
    {
        if (row < data_.size() && col < data_[row].size())
        {
            return data_[row][col];
        }
        else
        {
            return ""; // Return empty string for out-of-bounds access
        }
    }

    // Function to get the total number of columns in the CSV
    size_t getTotalColumns() const
    {
        if (!data_.empty())
        {
            return data_[0].size();
        }
        else
        {
            return 0; // Return 0 for an empty CSV
        }
    }

    // Function to get the total number of rows in the CSV
    size_t getTotalRows() const
    {
        return data_.size();
    }

    // Function to get the table header
    const std::vector<std::string> &getTableHeader() const
    {
        return tableHeader_;
    }

    // Function to get the header along with column indices
    std::vector<std::pair<std::string, size_t>> getHeaderWithIndices() const
    {
        std::vector<std::pair<std::string, size_t>> headerWithIndices;
        for (size_t i = 0; i < tableHeader_.size(); ++i)
        {
            headerWithIndices.emplace_back(tableHeader_[i], i);
        }
        return headerWithIndices;
    }

    // Function to clear loaded data
    void clearData()
    {
        data_.clear();
        tableHeader_.clear();
    }
};
