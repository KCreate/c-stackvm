#ifndef MACHINEH
#define MACHINEH

// Error codes
#define REGULAR_EXIT          0x00
#define ILLEGAL_MEMORY_ACCESS 0x01
#define INVALID_INSTRUCTION   0x02
#define INVALID_REGISTER      0x03
#define INVALID_SYSCALL       0x04
#define EXECUTABLE_TOO_BIG    0x05
#define INVALID_EXECUTABLE    0x06

// Registers
#define VM_REGCOUNT     64
#define VM_REGIP        60
#define VM_REGSP        61
#define VM_REGFP        62
#define VM_REGFLAGS     63

// Bitmasks for the flags register
#define VM_FLAG_ZERO    0

// Mode masks for register codes
#define VM_REGBYTE      192
#define VM_REGWORD      128
#define VM_REGDWORD     64
#define VM_REGQWORD     0

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
  op_syscall
} opcode;

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

// Header offsets
#define VM_HEADER_MINSIZE 12
#define VM_HEADER_MAGIC   0x4543494e

// Types used throughout the VM
typedef char byte;
typedef short word;
typedef int dword;
typedef long int qword;
typedef char opcode_t;
typedef char reg_t;
typedef int address_t;
typedef int offset_t;
typedef int sizespec_t;
typedef short syscall_t;

// The machine itself
typedef struct VM {
  byte* memory;
  qword* regs;
  bool running;
  byte exit_code;
} VM;

// An entry in the executables load table
typedef struct LoadEntry {
  address_t offset;
  sizespec_t size;
  address_t load;
} LoadEntry;

// The header of an executable
typedef struct Header {
  char magic[4];
  address_t entry_addr;
  int load_table_size;
  LoadEntry* load_table;
} Header;

// An executable for the vm
typedef struct Executable {
  Header* header;
  byte* data;
  sizespec_t data_size;
} Executable;

// VM Methods
bool vm_create(VM** vm);
void vm_clean(VM* vm);
bool vm_flash(Executable* exe);
bool vm_cycle(VM* vm);

// Executable methods
Executable* exe_create(char* buffer, size_t size);
void exe_clean(Executable* exe);

#endif
