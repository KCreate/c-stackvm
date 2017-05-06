#include <stdint.h>

#ifndef EXEH
#define EXEH

// Header offsets
#define EXE_HEADER_MINSIZE 12
#define EXE_HEADER_MAGIC   0x4543494e

// An entry in the executables load table
typedef struct LoadEntry {
  unsigned int offset;
  unsigned int size;
  unsigned int load;
} LoadEntry;

// The header of an executable
typedef struct Header {
  uint32_t entry_addr;
  size_t load_table_size;
  LoadEntry* load_table;
} Header;

// An executable for the vm
typedef struct Executable {
  Header* header;
  uint8_t* data;
  size_t data_size;
} Executable;

// Error codes for the exe_create function
typedef enum {
  exe_err_success,
  exe_err_too_small,
  exe_err_invalid_magicnum,
  exe_err_allocation
} ExecutableError;

// Executable methods
ExecutableError exe_create(Executable** result, uint8_t* buffer, size_t size);
char* exe_err(ExecutableError errcode);
void exe_print_info(Executable* exe);
void exe_clean(Executable* exe);

#endif
