#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vm.h"
#include "exe.h"

/*
 * Allocate the memory for VM struct
 * Returns false if allocation failed
 * */
VMError vm_create(VM** vm) {
  VM* vm_ptr = malloc(sizeof(VM));
  uint8_t* memory = malloc(VM_MEMORYSIZE * sizeof(uint8_t));
  uint64_t* regs = malloc(VM_REGCOUNT * sizeof(uint64_t));

  if (vm_ptr == NULL || memory == NULL || regs == NULL) {
    return vm_err_allocation;
  }

  vm_ptr->memory = memory;
  vm_ptr->regs = regs;
  vm_ptr->running = true;
  vm_ptr->exit_code = 0;

  *vm = vm_ptr;
  return vm_err_regular_exit;
}

/*
 * Clean the resources used by a vm struct
 * */
void vm_clean(VM* vm) {
  if (vm == NULL) return;

  free(vm->memory);
  free(vm->regs);
  return;
}

/*
 * Try to load a given executable into a virtual machine
 * */
VMError vm_flash(VM* vm, Executable* exe) {

  // Reset the machine
  memset(vm->regs, 0, VM_REGCOUNT * sizeof(uint64_t));
  memset(vm->memory, 0, VM_MEMORYSIZE);
  vm->running = true;
  vm->exit_code = 0;

  // Initialize special purpose registers
  vm->regs[VM_REGSP] = VM_STACK_START;
  vm->regs[VM_REGFP] = VM_MEMORYSIZE;
  vm->regs[VM_REGIP] = exe->header->entry_addr;

  // If the executables load table is empty
  // we assume that there is an entry which loads
  // the entire data segment onto address 0x00
  if (exe->header->load_table_size == 0) {
    if (exe->data_size > VM_MEMORYSIZE) {
      return vm_err_executable_too_big;
    }

    memcpy(vm->memory, exe->data, exe->data_size);
    return vm_err_regular_exit;
  }

  // Iterate over the load table and copy each segment
  // into it's specified location
  for (int i = 0; i < exe->header->load_table_size; i++) {
    LoadEntry entry = exe->header->load_table[i];

    // Check overflow in executable
    if (entry.offset + entry.size > exe->data_size) {
      return vm_err_invalid_executable;
    }

    // Check overflow for machine memory
    if (entry.load + entry.size > VM_MEMORYSIZE) {
      return vm_err_invalid_executable;
    }

    // Copy the relevant bytes into the machines memory
    memcpy(vm->memory + entry.load, exe->data + entry.offset, entry.size);
  }

  return vm_err_regular_exit;
}

/*
 * Perform a single cpu cycle in the vm
 * Returns false if no cycle could be performed
 * */
bool vm_cycle(VM* vm) {
  uint32_t ip = vm->regs[VM_REGIP];

  return true;
}

/*
 * Returns a human-readable version of an error code
 * */
char* vm_err(VMError errcode) {
  switch (errcode) {
    case vm_err_regular_exit:
      return "Success";
    case vm_err_illegal_memory_access:
      return "Illegal memory access";
    case vm_err_invalid_instruction:
      return "Invalid instruction";
    case vm_err_invalid_register:
      return "Invalid register";
    case vm_err_invalid_syscall:
      return "Invalid syscall";
    case vm_err_executable_too_big:
      return "Executable too big";
    case vm_err_invalid_executable:
      return "Invalid executable";
    case vm_err_allocation:
      return "Allocation failure";
    case vm_err_internal_failure:
      return "Internal failure";
    default:
      return "Unknown error";
  }
}
