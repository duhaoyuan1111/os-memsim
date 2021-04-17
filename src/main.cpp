#include <iostream>
#include <string>
#include <cstring>
#include <list>
#include <math.h>
#include <stdio.h>
#include "mmu.h"
#include "pagetable.h"

std::vector<uint32_t> processesRunningSoFar;

void printStartMessage(int page_size);
void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table);
void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table);
void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory);
void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table);
void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table);

int main(int argc, char **argv)
{
    // Ensure user specified page size as a command line parameter
    if (argc < 2)
    {
        fprintf(stderr, "Error: you must specify the page size\n");
        return 1;
    }

    // Print opening instuction message
    int page_size = std::stoi(argv[1]);
    printStartMessage(page_size);

    // Create physical 'memory'
    uint32_t mem_size = 67108864; // Bytes
    void *memory = malloc(mem_size); // 64 MB (64 * 1024 * 1024)

    // Create MMU and Page Table
    Mmu *mmu = new Mmu(mem_size);
    PageTable *page_table = new PageTable(page_size);

    // Prompt loop
    std::string command;
    std::cout << "> ";
    std::getline (std::cin, command);
    while (command != "exit") {
        std::vector<std::string> command_list;
        std::string del = " ";
        size_t pos = 0;
        std::string token;
        while ((pos = command.find(del)) != std::string::npos) {
            token = command.substr(0, pos);
            command_list.push_back(token);
            command.erase(0, pos + del.length());
        }
        command_list.push_back(command); // Get commands split by space
        // Handle command
        if (command_list[0] == "create") {
            int text_size = std::stoi(command_list[1]);
            int data_size = std::stoi(command_list[2]);
            //std::cout << text_size << std::endl;
            //std::cout << data_size << std::endl;
            createProcess(text_size, data_size, mmu, page_table);
        } else if (command_list[0] == "allocate") {
            uint32_t pid = static_cast<uint32_t>(std::stoul(command_list[1]));
            std::string var_name = command_list[2];
            std::string typeInString = command_list[3];
            DataType type;
            if (typeInString == "char") {
                type = Char;
            } else if (typeInString == "short") {
                type = Short;
            } else if (typeInString == "int") {
                type = Int;
            } else if (typeInString == "float") {
                type = Float;
            } else if (typeInString == "long") {
                type = Long;
            } else if (typeInString == "double") {
                type = Double;
            }
            uint32_t num_elements = static_cast<uint32_t>(std::stoul(command_list[4]));
            allocateVariable(pid, var_name, type, num_elements, mmu, page_table);
        } else if (command_list[0] == "set") {
            uint32_t pid = static_cast<uint32_t>(std::stoul(command_list[1]));
            std::string var_name = command_list[2];
            uint32_t offset = static_cast<uint32_t>(std::stoul(command_list[3]));
            if (!mmu->doWeHaveProcess(pid)) {
                // error: process not found
                std::cout << "error: process not found" << std::endl;
            } else if (!mmu->doWeHaveVariable(pid, var_name)) {
                // error: variable not found
                std::cout << "error: variable not found" << std::endl;
            } else {
                if (mmu->getVariableType(pid, var_name) == DataType::Char) {
                    for (int i = 4; i < command_list.size(); i++) {
                        char value = command_list[i][0];
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                } else if (mmu->getVariableType(pid, var_name) == DataType::Short) {
                    for (int i = 4; i < command_list.size(); i++) {
                        int temp = stoi(command_list[i]);
                        short value = (short)temp;
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                } else if (mmu->getVariableType(pid, var_name) == DataType::Int) {
                    for (int i = 4; i < command_list.size(); i++) {
                        int value = stoi(command_list[i]);
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                } else if (mmu->getVariableType(pid, var_name) == DataType::Float) {
                    for (int i = 4; i < command_list.size(); i++) {
                        float value = stof(command_list[i]);
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                } else if (mmu->getVariableType(pid, var_name) == DataType::Long) {
                    for (int i = 4; i < command_list.size(); i++) {
                        long value = stol(command_list[i]);
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                } else if (mmu->getVariableType(pid, var_name) == DataType::Double) {
                    for (int i = 4; i < command_list.size(); i++) {
                        double value = stod(command_list[i]);
                        setVariable(pid, var_name, offset, &value, mmu, page_table, memory);
                        offset++;
                    }
                }
            }
        } else if (command_list[0] == "free") {
            uint32_t pid = static_cast<uint32_t>(std::stoul(command_list[1]));
            std::string var_name = command_list[2];
            freeVariable(pid, var_name, mmu, page_table);
        } else if (command_list[0] == "terminate") {
            uint32_t pid = static_cast<uint32_t>(std::stoul(command_list[1]));
            terminateProcess(pid, mmu, page_table);
        } else if (command_list[0] == "print") {
            if (command_list[1] == "mmu") {
                mmu->print();
            } else if (command_list[1] == "page") {
                page_table->print();
            } else if (command_list[1] == "processes") {
                for (int j = 0; j < processesRunningSoFar.size(); j++) {
                    std::cout << processesRunningSoFar[j] << std::endl;
                }
            } else {
                std::vector<std::string> pidAndVar;
                std::string del2 = ":";
                size_t pos2 = 0;
                std::string token2;
                while ((pos2 = command_list[1].find(del2)) != std::string::npos) {
                    token2 = command_list[1].substr(0, pos2);
                    pidAndVar.push_back(token2);
                    command_list[1].erase(0, pos2 + del2.length());
                }
                pidAndVar.push_back(command_list[1]);
                uint32_t tempPid = static_cast<uint32_t>(std::stoul(pidAndVar[0]));
                Variable* tempVar = mmu->findVariable(tempPid, pidAndVar[1]);
                int phys_addr = page_table->getPhysicalAddress(tempPid, tempVar->virtual_address);
                if (tempVar->type == DataType::Char) {
                    char tempCharArray[tempVar->size];
                    memcpy(tempCharArray, (char*)memory + phys_addr, tempVar->size);
                    if (tempVar->size > 4) { // do we have more than 4 items?
                        printf("%c", tempCharArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %c", tempCharArray[h]);
                        }
                        printf(", ... [%ul items]\n", tempVar->size);
                    } else {
                        printf("%c", tempCharArray[0]);
                        for (int h = 1; h < tempVar->size; h++) { // print first 4 items
                            printf(", %c", tempCharArray[h]);
                        }
                        printf("\n");
                    }
                } else if (tempVar->type == DataType::Short) {
                    uint32_t items = tempVar->size / 2;
                    short tempShortArray[items];
                    memcpy(tempShortArray, (char*)memory + phys_addr, tempVar->size);
                    if (items > 4) { // do we have more than 4 items?
                        printf("%hd", tempShortArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %hd", tempShortArray[h]);
                        }
                        printf(", ... [%ul items]\n", items);
                    } else {
                        printf("%hd", tempShortArray[0]);
                        for (int h = 1; h < items; h++) { // print first 4 items
                            printf(", %hd", tempShortArray[h]);
                        }
                        printf("\n");
                    }
                } else if (tempVar->type == DataType::Int) {
                    uint32_t items = tempVar->size / 4;
                    int tempIntArray[items];
                    memcpy(tempIntArray, (char*)memory + phys_addr, tempVar->size);
                    if (items > 4) { // do we have more than 4 items?
                        printf("%d", tempIntArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %d", tempIntArray[h]);
                        }
                        printf(", ... [%ul items]\n", items);
                    } else {
                        printf("%d", tempIntArray[0]);
                        for (int h = 1; h < items; h++) { // print first 4 items
                            printf(", %d", tempIntArray[h]);
                        }
                        printf("\n");
                    }
                } else if (tempVar->type == DataType::Float) {
                    uint32_t items = tempVar->size / 4;
                    float tempFloatArray[items];
                    memcpy(tempFloatArray, (char*)memory + phys_addr, tempVar->size);
                    if (items > 4) { // do we have more than 4 items?
                        printf("%f", tempFloatArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %f", tempFloatArray[h]);
                        }
                        printf(", ... [%ul items]\n", items);
                    } else {
                        printf("%f", tempFloatArray[0]);
                        for (int h = 1; h < items; h++) { // print first 4 items
                            printf(", %f", tempFloatArray[h]);
                        }
                        printf("\n");
                    }
                } else if (tempVar->type == DataType::Long) {
                    uint32_t items = tempVar->size / 8;
                    long tempLongArray[items];
                    memcpy(tempLongArray, (char*)memory + phys_addr, tempVar->size);
                    if (items > 4) { // do we have more than 4 items?
                        printf("%ld", tempLongArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %ld", tempLongArray[h]);
                        }
                        printf(", ... [%ul items]\n", items);
                    } else {
                        printf("%ld", tempLongArray[0]);
                        for (int h = 1; h < items; h++) { // print first 4 items
                            printf(", %ld", tempLongArray[h]);
                        }
                        printf("\n");
                    }
                } else if (tempVar->type == DataType::Double) {
                    uint32_t items = tempVar->size / 8;
                    double tempDoubleArray[items];
                    memcpy(tempDoubleArray, (char*)memory + phys_addr, tempVar->size);
                    if (items > 4) { // do we have more than 4 items?
                        printf("%f", tempDoubleArray[0]);
                        for (int h = 1; h < 4; h++) { // print first 4 items
                            printf(", %f", tempDoubleArray[h]);
                        }
                        printf(", ... [%ul items]\n", items);
                    } else {
                        printf("%f", tempDoubleArray[0]);
                        for (int h = 1; h < items; h++) { // print first 4 items
                            printf(", %f", tempDoubleArray[h]);
                        }
                        printf("\n");
                    }
                }
                
            }

        }
        // Get next command
        std::cout << "> ";
        std::getline (std::cin, command);
    }

    // Clean up
    free(memory);
    delete mmu;
    delete page_table;

    return 0;
}

void printStartMessage(int page_size)
{
    std::cout << "Welcome to the Memory Allocation Simulator! Using a page size of " << page_size << " bytes." << std:: endl;
    std::cout << "Commands:" << std:: endl;
    std::cout << "  * create <text_size> <data_size> (initializes a new process)" << std:: endl;
    std::cout << "  * allocate <PID> <var_name> <data_type> <number_of_elements> (allocated memory on the heap)" << std:: endl;
    std::cout << "  * set <PID> <var_name> <offset> <value_0> <value_1> <value_2> ... <value_N> (set the value for a variable)" << std:: endl;
    std::cout << "  * free <PID> <var_name> (deallocate memory on the heap that is associated with <var_name>)" << std:: endl;
    std::cout << "  * terminate <PID> (kill the specified process)" << std:: endl;
    std::cout << "  * print <object> (prints data)" << std:: endl;
    std::cout << "    * If <object> is \"mmu\", print the MMU memory table" << std:: endl;
    std::cout << "    * if <object> is \"page\", print the page table" << std:: endl;
    std::cout << "    * if <object> is \"processes\", print a list of PIDs for processes that are still running" << std:: endl;
    std::cout << "    * if <object> is a \"<PID>:<var_name>\", print the value of the variable for that process" << std:: endl;
    std::cout << std::endl;
}

void createProcess(int text_size, int data_size, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    uint32_t pid;
    uint32_t stack_size = 65536;
    std::string text = "<TEXT>";
    std::string globals = "<GLOBALS>";
    std::string stack = "<STACK>";
    //   - create new process in the MMU
    pid = mmu->createProcess();
    //   - allocate new variables for the <TEXT>, <GLOBALS>, and <STACK>
    allocateVariable(pid, text, DataType::Char, (uint32_t)text_size, mmu, page_table);
    allocateVariable(pid, globals, DataType::Char, (uint32_t)data_size, mmu, page_table);
    allocateVariable(pid, stack, DataType::Char, stack_size, mmu, page_table);
    //   - print pid
    std::cout << pid << std::endl;
}

void allocateVariable(uint32_t pid, std::string var_name, DataType type, uint32_t num_elements, Mmu *mmu, PageTable *page_table)
{
    // Get the total size of this new var
    uint32_t sizeInTotal;
    int sizeOfType;
    if (DataType::Char == type) {
        sizeInTotal = num_elements * 1;
        sizeOfType = 1;
    } else if (DataType::Short == type) {
        sizeInTotal = num_elements * 2;
        sizeOfType = 2;
    } else if (DataType::Int == type || DataType::Float == type) {
        sizeInTotal = num_elements * 4;
        sizeOfType = 4;
    } else if (DataType::Double == type || DataType::Long == type) {
        sizeInTotal = num_elements * 8;
        sizeOfType = 8;
    }
    // Get variableList
    std::vector<Variable*> variableList = mmu->getVariableList(pid);
    int idxToInsert = -1;
    // Loop through the varList to find the middle spot that next to <free space>
    for (int i = 0; i < variableList.size(); i++) {
        if (variableList[i]->name == "<FREE_SPACE>") {
            if (variableList[i]->size >= sizeInTotal) {
                idxToInsert = i;
                break;
            }
        }
    }

    if (idxToInsert == -1) {
        // no free space in that process which means it exceeds 64 MB
        // error
        return;
    }

    double start_page_double;
    double end_page_double;
    int start_page_int;
    int end_page_int;

    // VariableList looks like: [<TEXT>, <GLOBALS>, <STACK>, thisIsAnInt, <FREE_SPACE>]
    //                                                                   ^
    //                                                   each time we insert the new var here
    if (idxToInsert != 0) { // if the new var has a neighbor on its left
        if (sizeInTotal > (variableList[idxToInsert-1]->size + variableList[idxToInsert-1]->virtual_address) % page_table->getPageSize()) {
            start_page_double = ((double)variableList[idxToInsert-1]->size + (double)variableList[idxToInsert-1]->virtual_address) / (double)page_table->getPageSize();
            start_page_int = floor(start_page_double); // index, 0 ~ n
            end_page_double = ((double)variableList[idxToInsert-1]->size + (double)variableList[idxToInsert-1]->virtual_address + (double)sizeInTotal) / (double)page_table->getPageSize();
            end_page_int = floor(end_page_double); // index, 0 ~ n
            int leftover = (variableList[idxToInsert-1]->size + variableList[idxToInsert-1]->virtual_address) % page_table->getPageSize();
            if (leftover % sizeOfType != 0) { // if leftover can't be divided with no remainder by type size
                uint32_t shortSpaceSize = leftover % sizeOfType;
                Variable *newFreeSpace = new Variable();
                // we get a small hole in between
                newFreeSpace->name = "<FREE_SPACE>";
                newFreeSpace->size = shortSpaceSize;
                newFreeSpace->type = DataType::Char;
                newFreeSpace->virtual_address = variableList[idxToInsert-1]->size + variableList[idxToInsert-1]->virtual_address;
                // update free space on right side
                variableList[idxToInsert]->size -= shortSpaceSize;
                variableList[idxToInsert]->virtual_address += shortSpaceSize;
                // insert the small hole
                variableList.insert(variableList.begin() + idxToInsert, newFreeSpace);
                idxToInsert++; // go right by 1 index
            }
            // find rest of pages whether have been on the book
            for (int i = start_page_int + 1; i <= end_page_int; i++) {
                if(!page_table->lookUpTable(pid, i)) { // if false, need to create new pages
                    page_table->addEntry(pid, i);
                }
            }
            Variable *newVar = new Variable();
            newVar->name = var_name;
            newVar->size = sizeInTotal;
            newVar->type = type;
            newVar->virtual_address = variableList[idxToInsert-1]->size + variableList[idxToInsert-1]->virtual_address;
            // update the right neighbor
            variableList[idxToInsert]->size -= sizeInTotal;
            variableList[idxToInsert]->virtual_address += sizeInTotal;
            // insert new var
            variableList.insert(variableList.begin() + idxToInsert, newVar);
            if (newVar->name != "<TEXT>" && newVar->name != "<GLOBALS>" &&  newVar->name != "<STACK>") {
                std::cout << newVar->virtual_address << std::endl;
            }
        } else {
            // consume less than a page
            Variable *newVar = new Variable();
            newVar->name = var_name;
            newVar->size = sizeInTotal;
            newVar->type = type;
            newVar->virtual_address = variableList[idxToInsert-1]->size + variableList[idxToInsert-1]->virtual_address;
            // update the right neighbor
            variableList[idxToInsert]->size -= sizeInTotal;
            variableList[idxToInsert]->virtual_address += sizeInTotal;
            // insert new var
            variableList.insert(variableList.begin() + idxToInsert, newVar);
            if (newVar->name != "<TEXT>" && newVar->name != "<GLOBALS>" &&  newVar->name != "<STACK>") {
                std::cout << newVar->virtual_address << std::endl;
            }
        }
    } else if (idxToInsert == 0) { // the new var is the most left one
        // Then it must be <TEXT>
        // It's an empty book, so just create pages for it
        end_page_double = (double)sizeInTotal / (double)page_table->getPageSize();
        start_page_int = 0;
        end_page_int = floor(end_page_double);
        for (int i = start_page_int + 1; i <= end_page_int; i++) { 
            if(!page_table->lookUpTable(pid, i)) {
                page_table->addEntry(pid, i);
            } else {
                std::cout << "--- We got a bad bug ---" << std::endl;
            }
        }
        Variable *newVar = new Variable();
        newVar->name = var_name;
        newVar->size = sizeInTotal;
        newVar->type = type;
        newVar->virtual_address = 0;
        // update the right neighbor
        variableList[idxToInsert]->size -= sizeInTotal;
        variableList[idxToInsert]->virtual_address += sizeInTotal;
        // insert new var
        variableList.insert(variableList.begin() + idxToInsert, newVar);
        if (newVar->name != "<TEXT>" && newVar->name != "<GLOBALS>" &&  newVar->name != "<STACK>") {
            std::cout << newVar->virtual_address << std::endl;
        }
    }

}

void setVariable(uint32_t pid, std::string var_name, uint32_t offset, void *value, Mmu *mmu, PageTable *page_table, void *memory)
{
    // TODO: implement this!
    //   - look up physical address for variable based on its virtual address / offset
    //   - insert `value` into `memory` at physical address
    //   * note: this function only handles a single element (i.e. you'll need to call this within a loop when setting
    //           multiple elements of an array) 
}

void freeVariable(uint32_t pid, std::string var_name, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove entry from MMU
    //   - free page if this variable was the only one on a given page
}

void terminateProcess(uint32_t pid, Mmu *mmu, PageTable *page_table)
{
    // TODO: implement this!
    //   - remove process from MMU
    //   - free all pages associated with given process
}
