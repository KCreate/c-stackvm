#include <stdbool.h>
#include <stdint.h>
#include "exe.h"

#ifndef VMH
#define VMH

// Error codes
#define REGULAR_EXIT          0x00
#define ILLEGAL_MEMORY_ACCESS 0x01
#define INVALID_INSTRUCTION   0x02
#define INVALID_REGISTER      0x03
#define INVALID_SYSCALL       0x04
#define EXECUTABLE_TOO_BIG    0x05
#define INVALID_EXECUTABLE    0x06
#define ALLOCATION_FAILURE    0x07

// Bitmasks for the flags register
#define VM_FLAG_ZERO    1

// Mode masks for register codes
#define VM_MODEMASK     192
#define VM_CODEMASK     63
#define VM_REGBYTE      192
#define VM_REGWORD      128
#define VM_REGDWORD     64
#define VM_REGQWORD     0

// Registers
#define VM_REGCOUNT     64
#define VM_REGIP        60 | VM_REGDWORD
#define VM_REGSP        61 | VM_REGDWORD
#define VM_REGFP        62 | VM_REGDWORD
#define VM_REGFLAGS     63 | VM_REGBYTE

// Opcodes
typedef enum opcode {
  op_rpush,
  op_rpop,
  op_mov,
  op_loadi,
  op_rst,

  op_add,
  op_sub,
  op_mul,
  op_div,
  op_idiv,
  op_rem,
  op_irem,

  op_fadd,
  op_fsub,
  op_fmul,
  op_fdiv,
  op_frem,
  op_fexp,

  op_flt,
  op_fgt,

  op_cmp,
  op_lt,
  op_gt,
  op_ult,
  op_ugt,

  op_shr,
  op_shl,
  op_and,
  op_xor,
  op_or,
  op_not,

  op_inttofp,
  op_sinttofp,
  op_fptoint,

  op_load,
  op_loadr,
  op_loads,
  op_loadsr,
  op_store,
  op_push,

  op_read,
  op_readc,
  op_reads,
  op_readcs,
  op_write,
  op_writec,
  op_writes,
  op_writecs,
  op_copy,
  op_copyc,

  op_jz,
  op_jzr,
  op_jmp,
  op_jmpr,
  op_call,
  op_callr,
  op_ret,

  op_nop,
  op_syscall,

  // This is here so we can compare an integer against it and
  // thus check if it's a valid instruction
  op_num_types
} opcode;

/*
 * Contains the amounts of bytes each opcode takes up
 * */
uint64_t opcode_length_lookup_table[59];

// Syscall ids
#define VM_SYS_EXIT   0x00
#define VM_SYS_SLEEP  0x01
#define VM_SYS_WRITE  0x02
#define VM_SYS_PUTS   0x03

// Well-known addresses
#define VM_STACK_START 0x00400000
#define VM_INTERNALS   0x00400000
#define VM_INT_HANDLER 0x00797bea
#define VM_INT_MEMORY  0x00979bee
#define VM_INT_CODE    0x00797bfe
#define VM_INT_STATUS  0x00797bff
#define VM_VRAM        0x00797c00

// Sizes of different things
#define VM_MEMORYSIZE     8000000     // 8 megabytes
#define VM_STACKSIZE      3572754
#define VM_INTERNALSSIZE  3767274
#define VM_INT_MEMORYSIZE 16
#define VM_VRAMSIZE       38400
#define VM_VRAMWIDTH      240
#define VM_VRAMHEIGHT     160

// The machine itself
typedef struct VM {
  uint8_t* memory;
  uint64_t* regs;
  bool running;
  uint8_t exit_code;
} VM;

typedef enum {
  vm_err_regular_exit,
  vm_err_illegal_memory_access,
  vm_err_invalid_instruction,
  vm_err_invalid_register,
  vm_err_invalid_syscall,
  vm_err_executable_too_big,
  vm_err_invalid_executable,
  vm_err_allocation,
  vm_err_internal_failure
} VMError;

// VM Methods
VMError vm_create(VM** vm);
void vm_clean(VM* vm);
VMError vm_flash(VM* vm, Executable* exe);
int vm_run(VM* vm, int* exit_code);
bool vm_cycle(VM* vm);
void vm_execute(VM* vm, opcode instruction, uint32_t ip);
uint64_t vm_instruction_length(VM* vm, opcode instruction);
char* vm_err(VMError errcode);
uint32_t vm_reg_size(uint8_t reg);
void vm_stack_write(VM* vm, uint32_t address, uint32_t size);
void vm_stack_write_block(VM* vm, void* block, size_t size);
void* vm_stack_pop(VM* vm, uint32_t size);
void vm_write_reg(VM* vm, uint8_t reg, uint64_t value);
uint64_t vm_read_reg(VM* vm, uint8_t reg);
void vm_move_mem_to_reg(VM* vm, uint8_t reg, uint32_t address, uint32_t size);
bool vm_legal_address(uint32_t address);

#endif
