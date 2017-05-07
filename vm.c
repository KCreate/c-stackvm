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
  vm_write_reg(vm, VM_REGSP, VM_STACK_START);
  vm_write_reg(vm, VM_REGFP, VM_MEMORYSIZE);
  vm_write_reg(vm, VM_REGIP, exe->header->entry_addr);

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
  uint32_t ip = vm_read_reg(vm, VM_REGIP);

  // Check if ip is out-of-bounds
  if (!vm_legal_address(ip)) {
    vm->exit_code = ILLEGAL_MEMORY_ACCESS;
    vm->running = false;
    return false;
  }

  opcode instruction = vm->memory[ip];

  uint64_t instruction_length = vm_instruction_length(vm, instruction);

  // Check if there is enough memory for the instruction
  if (!vm_legal_address(ip + instruction_length)) {
    vm->exit_code = ILLEGAL_MEMORY_ACCESS;
    vm->running = false;
    return false;
  }

  vm_execute(vm, instruction, ip);

  // If the instruction we just executed didn't change the instruction pointer
  // we increment it to the next instruction
  //
  // Since our instruction format isn't of fixed length, we have to calculate
  // the offset to the next instruction. For most instructions this is a simple
  // table-lookup, only loadi and push require a custom calculation
  if (ip == vm_read_reg(vm, VM_REGIP)) {
    vm_write_reg(vm, VM_REGIP, ip + instruction_length);
  }

  return true;
}

/*
 * Execute an instruction
 * */
void vm_execute(VM* vm, opcode instruction, uint32_t ip) {
  switch (instruction) {
    default:
      vm->exit_code = INVALID_INSTRUCTION;
      vm->running = false;
      return;
  }
}

/*
 * Calculate the length of a given instruction at the current instruction pointer
 * in the virtual machine
 * */
uint64_t vm_instruction_length(VM* vm, opcode instruction) {
  switch (instruction) {
    case op_loadi: {
      uint32_t ip = vm_read_reg(vm, VM_REGIP);
      uint8_t reg = *(uint8_t *)(vm->memory + ip + 1);

      //     +- Opcode
      //     |   +- Register code
      //     |   |   +- Immediate value
      //     |   |   |
      //     v   v   v
      return 1 + 1 + vm_reg_size(reg);
    }
    case op_push: {
      uint32_t ip = vm_read_reg(vm, VM_REGIP);
      uint32_t size = *(uint32_t *)(vm->memory + ip + 1);

      //     +- Opcode
      //     |   +- Size specifier
      //     |   |   +- Immediate value
      //     |   |   |
      //     v   v   v
      return 1 + 4 + size;
    }
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
 * Return the size of a given register
 * */
uint32_t vm_reg_size(uint8_t reg) {
  switch (reg & VM_MODEMASK) {
    case VM_REGBYTE:
      return 1;
    case VM_REGWORD:
      return 2;
    case VM_REGDWORD:
      return 4;
    case VM_REGQWORD:
      return 8;
    default:
      return 0; // Can't happen
  }
}

/*
 * Write a block of memory onto the stack
 * */
void vm_stack_write(VM* vm, uint32_t address, uint32_t size) {
  uint32_t sp = vm_read_reg(vm, VM_REGSP);

  // Check for a stack underflow
  if (sp < size || address + size - 1 >= VM_MEMORYSIZE) {
    vm->exit_code = ILLEGAL_MEMORY_ACCESS;
    vm->running = false;
    return;
  }

  memcpy(vm->memory + sp - size, vm->memory + address, size);
  vm_write_reg(vm, VM_REGSP, sp - size);
}

/*
 * Pop some bytes off the stack and return a pointer
 * to the bytes which were just popped off
 * */
uint32_t vm_stack_pop(VM* vm, uint32_t size) {
  uint32_t sp = vm_read_reg(vm, VM_REGSP);

  // Check for a stack underflow
  if (sp >= VM_MEMORYSIZE || sp > size) {
    vm->exit_code = ILLEGAL_MEMORY_ACCESS;
    vm->running = false;
    return 0;
  }

  vm_write_reg(vm, VM_REGSP, sp + size);
  return sp;
}

/*
 * Write a value into a register
 * */
void vm_write_reg(VM* vm, uint8_t reg, uint64_t value) {
  switch (vm_reg_size(reg)) {
    case 1:
      *((uint8_t *) (vm->regs + (reg & VM_CODEMASK))) = (uint8_t) value;
      break;
    case 2:
      *((uint16_t *) (vm->regs + (reg & VM_CODEMASK))) = (uint16_t) value;
      break;
    case 4:
      *((uint32_t *) (vm->regs + (reg & VM_CODEMASK))) = (uint32_t) value;
      break;
    case 8:
      *((uint64_t *) (vm->regs + (reg & VM_CODEMASK))) = (uint64_t) value;
      break;
    default:
      break; // can't happen
  }
}

/*
 * Moves a block of memory into a register
 * */
void vm_move_mem_to_reg(VM* vm, uint8_t reg, uint32_t address, uint32_t size) {
  if (VM_MEMORYSIZE - size < address) {
    vm->exit_code = ILLEGAL_MEMORY_ACCESS;
    vm->running = false;
    return;
  }

  switch (size) {
    case 1:
      vm_write_reg(vm, reg, (uint8_t) vm->memory + address);
      break;
    case 2:
      vm_write_reg(vm, reg, (uint16_t) vm->memory + address);
      break;
    case 4:
      vm_write_reg(vm, reg, (uint32_t) vm->memory + address);
      break;
    default:
      vm_write_reg(vm, reg, (uint64_t) vm->memory + address);
      break;
  }
}

/*
 * Read the value of a register
 * */
uint64_t vm_read_reg(VM* vm, uint8_t reg) {
  switch (vm_reg_size(reg)) {
    case 1:
      return *(uint64_t *)(uint8_t *) (vm->regs + (reg & VM_CODEMASK));
    case 2:
      return *(uint64_t *)(uint16_t *) (vm->regs + (reg & VM_CODEMASK));
    case 4:
      return *(uint64_t *)(uint32_t *) (vm->regs + (reg & VM_CODEMASK));
    case 8:
      return vm->regs[reg & VM_CODEMASK];
    default:
      return 0; // can't happen
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
