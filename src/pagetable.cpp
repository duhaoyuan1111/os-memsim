#include "pagetable.h"

PageTable::PageTable(int page_size)
{
    _page_size = page_size;
}

PageTable::~PageTable()
{
}

std::vector<std::string> PageTable::sortedKeys()
{
    std::vector<std::string> keys;

    std::map<std::string, int>::iterator it;
    for (it = _table.begin(); it != _table.end(); it++)
    {
        keys.push_back(it->first);
    }

    std::sort(keys.begin(), keys.end(), PageTableKeyComparator());

    return keys;
}

void PageTable::addEntry(uint32_t pid, int page_number)
{
    // Combination of pid and page number act as the key to look up frame number
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);

    int frame = 0;
    // Find free frame
    if (!_table.empty()) { // not empty
        for (auto const& x : _table) {
            if (x.second >= frame) {
                frame = x.second;
            }
        }
        frame++; // next valid frame
        _table[entry] = frame;
    } else { // empty, use frame 0
        _table[entry] = frame;
    }
}

int PageTable::getPhysicalAddress(uint32_t pid, uint32_t virtual_address)
{
    // Convert virtual address to page_number and page_offset
    int page_number = virtual_address / _page_size;
    int page_offset = virtual_address % _page_size;

    // Combination of pid and page number act as the key to look up frame number
    // !!! We are using frame number here !!!
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    
    // If entry exists, look up frame number and convert virtual to physical address
    int address = -1;
    if (_table.count(entry) > 0)
    {
        address = _table[entry] * _page_size + page_offset;
    }

    return address;
}

void PageTable::print()
{
    int i;

    std::cout << " PID  | Page Number | Frame Number" << std::endl;
    std::cout << "------+-------------+--------------" << std::endl;

    std::vector<std::string> keys = sortedKeys();

    for (i = 0; i < keys.size(); i++)
    {
        std::string del = "|";
        std::string temp = keys.at(i);
        int frame_number = _table[keys.at(i)];
        size_t pos = temp.find(del);
        std::string pid = temp.substr(0, pos);
        temp.erase(0, (pos + del.length()));
        int temp2 = stoi(temp);
        temp2--;
        printf(" %4s | %11d | %12d \n", pid.c_str(), temp2, frame_number);
    }
}

int PageTable::getPageSize() {
    return _page_size;
}

bool PageTable::lookUpTable(uint32_t pid, int page_number) {
    std::string entry = std::to_string(pid) + "|" + std::to_string(page_number);
    if (_table.count(entry) > 0) {
        // found
        return true;
    } else {
        // not existed yet
        return false;
    }
}
