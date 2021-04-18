#include "mmu.h"

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
        //push_back(var);
        std::cout << var->virtual_address << "---vir_addr" << std::endl;
        std::cout << var->name << "---name" << std::endl;
        std::cout << proc->variables.size() << "---size"<< std::endl;
        std::cout << var->virtual_address + var->size << "---ending addr"<< std::endl;
    }
}

void Mmu::print()
{
    int i, j;

    std::cout << " PID  | Variable Name | Virtual Addr | Size" << std::endl;
    std::cout << "------+---------------+--------------+------------" << std::endl;
    for (i = 0; i < _processes.size(); i++)
    {
        for (j = 0; j < _processes[i]->variables.size(); j++)
        {
            // TODO: print all variables (excluding <FREE_SPACE> entries)
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
