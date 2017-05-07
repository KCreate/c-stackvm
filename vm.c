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
    if (!vm_legal_address(exe->data_size)) {
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
    if (!vm_legal_address(entry.load + entry.size)) {
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
 * Calculate the length of a given instruction at the current instruction pointer
 * in the virtual machine
 * */
uint64_t vm_instruction_length(VM* vm, opcode instruction) {
  switch (instruction) {
    case op_loadi:
    case op_push:
      fprintf(stderr, "length decoding for loadi and push is not implemented yet");
      exit(1);
    default:

      // Check if this is a valid instruction
      // If not we just return 1 to jump over it
      if (instruction >= op_num_types) {
        return 1;
      }

      return opcode_length_lookup_table[instruction];
  }
}

/*
 * Returns true if address is legal
 * */
bool vm_legal_address(uint32_t address) {
  return address < VM_MEMORYSIZE;
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

/*
 * Define the instruction lengths for all opcodes
 * */
uint64_t opcode_length_lookup_table[59] = {
  2, // rpush
  2, // rpop
  3, // mov
  0, // loadi (calculated in the vm itself)
  2, // rst

  3, // add
  3, // sub
  3, // mul
  3, // div
  3, // idiv
  3, // rem
  3, // irem

  3, // fadd
  3, // fsub
  3, // fmul
  3, // fdiv
  3, // frem
  3, // fexp

  3, // flt
  3, // fgt

  3, // cmp
  3, // lt
  3, // gt
  3, // ult
  3, // ugt

  3, // shr
  3, // shl
  3, // and
  3, // xor
  3, // or
  2, // not

  2, // inttofp
  2, // sinttofp
  2, // fptoint

  6, // load
  3, // loadr
  9, // loads
  6, // loadsr
  6, // store
  0, // push (this is calculated in the vm itself)

  3,  // read
  6,  // readc
  6,  // reads
  9,  // readcs
  3,  // write
  6,  // writec
  6,  // writes
  9,  // writecs
  7,  // copy
  13, // copyc

  5, // jz
  2, // jzr
  5, // jmp
  2, // jmpr
  5, // call
  2, // callr
  1, // ret

  1, // nop
  1, // syscall
};
