#include "mmu.h"
#include <math.h>

Mmu::Mmu(int memory_size)
{
    _next_pid = 1024;
    _max_size = memory_size;
}

Mmu::~Mmu()
{
}

uint32_t Mmu::createProcess()
{
    Process *proc = new Process();
    proc->pid = _next_pid;

    Variable *var = new Variable();
    var->name = "<FREE_SPACE>";
    var->type = DataType::FreeSpace;
    var->virtual_address = 0;
    var->size = _max_size;
    proc->variables.push_back(var);

    _processes.push_back(proc);

    _next_pid++;
    return proc->pid;
}

void Mmu::addVariableToProcess(uint32_t pid, std::string var_name, DataType type, uint32_t size, uint32_t address, int idxToInsert)
{
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }

    // Print error message if an allocation would exceed system memory (and don't perform allocation)
    uint32_t cal = 0;
    if (idxToInsert == proc->variables.size() - 1) { // if insert at the end
        for (int j = 0; j < _processes.size(); j++) {
            cal += _processes[j]->variables[_processes[j]->variables.size() - 1]->virtual_address;
        }
    }
    cal += size;
    if (cal > 67108864) {
        std::cout << "error: this allocation would exceed system memory" << std::endl;
        return;
    }

    Variable *var = new Variable();
    var->name = var_name;
    var->type = type;
    var->virtual_address = address;
    var->size = size;
    if (proc != NULL)
    {
        proc->variables[idxToInsert]->size -= size;
        proc->variables[idxToInsert]->virtual_address += size;
        proc->variables.insert(proc->variables.begin() + idxToInsert, var);
        
        if (var_name != "<TEXT>" && var_name != "<GLOBALS>" && var_name != "<STACK>" && var_name != "<FREE_SPACE>") {
            std::cout << var->virtual_address << std::endl;
        }
        /*std::cout << var->name << "---name" << std::endl;
        std::cout << proc->variables.size() << "---size"<< std::endl;
        std::cout << var->virtual_address + var->size << "---ending addr"<< std::endl;*/
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        uint32_t pid = _processes[i]->pid;
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            std::string var_name = _processes[i]->variables[j]->name;
            uint32_t vir_addr = _processes[i]->variables[j]->virtual_address;
            uint32_t var_size = _processes[i]->variables[j]->size;
            if (var_name != "<FREE_SPACE>") {
                printf(" %4u | %-13s |  0x%08X  | %10u \n", pid, var_name.c_str(), vir_addr, var_size);
            }
        }
    }
}

DataType Mmu::getVariableType(uint32_t pid, std::string var_name) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    for (int j = 0; j < proc->variables.size(); j++) {
        if (proc->variables[j]->name == var_name)
        {
            return proc->variables[j]->type;
        }
    }
    std::cout << "We got a bug in Mmu::getVariableType." << std::endl;
    return FreeSpace;
}

bool Mmu::doWeHaveProcess(uint32_t pid) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    if (proc == NULL) {
        return false;
    } else {
        return true;
    }
}

bool Mmu::doWeHaveVariable(uint32_t pid, std::string var_name) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    for (int j = 0; j < proc->variables.size(); j++) {
        if (proc->variables[j]->name == var_name)
        {
            return true;
        }
    }
    return false;
}

Variable* Mmu::findVariable(uint32_t pid, std::string var_name) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    for (int j = 0; j < proc->variables.size(); j++) {
        if (proc->variables[j]->name == var_name)
        {
            return proc->variables[j];
        }
    }
    std::cout << "We got a bug in Mmu::findVariable." << std::endl;
    return NULL;
}

std::vector<Variable*> Mmu::getVariableList(uint32_t pid) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    return proc->variables;
}

void Mmu::printProcesses() {
    for (int i = 0; i < _processes.size(); i++) {
        std::cout << _processes[i]->pid << std::endl;
    }
}

void Mmu::removeVariableFromProcess(uint32_t pid, std::string var_name) {
    int i;
    Process *proc = NULL;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    for (int j = 0; j < proc->variables.size(); j++) {
        if (proc->variables[j]->name == var_name)
        {
            proc->variables[j]->name = "<FREE_SPACE>";
        }
    }
}

std::vector<int> Mmu::mergeFreeSpace(uint32_t pid, int page_size) {
    int i;
    Process *proc = NULL;
    std::vector<int> retVec;
    for (i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            proc = _processes[i];
        }
    }
    int j = 0;
    while (j < proc->variables.size() - 1) { // merge
        int flag = 0;
        if (proc->variables[j]->name == "<FREE_SPACE>" && proc->variables[j+1]->name == "<FREE_SPACE>") {
            proc->variables[j]->size += proc->variables[j+1]->size;
            flag = 1;
        }
        if (flag == 1) {
            proc->variables.erase(proc->variables.begin() + j + 1); // delete right one
        } else {
            j++;
        }
    }
    for (int k = 0; k < proc->variables.size(); k++) { // pages need to be deleted
        if (proc->variables[k]->name == "<FREE_SPACE>") {
            double start_page_double;
            double end_page_double;
            int start_page_int;
            int end_page_int;
            start_page_double = ((double)proc->variables[k]->virtual_address) / (double)page_size;
            start_page_int = floor(start_page_double); // index, 0 ~ n
            end_page_double = ((double)proc->variables[k]->size + (double)proc->variables[k]->virtual_address) / (double)page_size;
            end_page_int = floor(end_page_double); // index, 0 ~ n
            if (start_page_int + 1 < end_page_int) {
                for (int d = start_page_int + 1; d < end_page_int; d++) {
                    retVec.push_back(d);
                }
            }
            if ((proc->variables[k]->virtual_address % page_size == 0) && (start_page_int != end_page_int)) { // start at the beginning of the page
                retVec.push_back(start_page_int);
            }
            if (((proc->variables[k]->virtual_address + proc->variables[k]->size) % page_size == 0) && (start_page_int != end_page_int)) { // end at the end of the page
                retVec.push_back(end_page_int);
            }
        }
    }
    if (retVec.empty()) {
        retVec.push_back(-1);
        return retVec;
    } else {
        return retVec;
    }
}

void Mmu::removeProcessFromMmu(uint32_t pid) {
    for (int i = 0; i < _processes.size(); i++)
    {
        if (_processes[i]->pid == pid)
        {
            _processes.erase(_processes.begin() + i);
        }
    }
}
