#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "exe.h"

/*
 * Parse an executable from buffer
 * */
ExecutableError exe_create(Executable** result, uint8_t* buffer, size_t size) {

  // Make sure the specified buffer is big enough to contain the
  // minimum neccessary fields
  if (size < EXE_HEADER_MINSIZE) {
    return exe_err_too_small;
  }

  // Check the magic number
  if (buffer[0] != 'N' || buffer[1] != 'I' || buffer[2] != 'C' || buffer[3] != 'E') {
    return exe_err_invalid_magicnum;
  }

  // Allocate and initialize the Header struct
  Header* header = calloc(1, sizeof(Header));
  if (!header) {
    return exe_err_allocation;
  }

  // Read the entry address and the load table size
  uint32_t entry_addr = *((uint32_t *) buffer + 1);
  size_t load_table_size = *((uint32_t *) buffer + 2);

  header->entry_addr = entry_addr;
  header->load_table_size = load_table_size;

  // Check if there is enough memory for the load table
  if ((size - 12) < load_table_size * 12) {
    return exe_err_too_small;
  }

  // Allocate space for the load table
  LoadEntry* load_table = malloc(load_table_size * sizeof(LoadEntry));
  if (!load_table) {
    return exe_err_allocation;
  }

  header->load_table = load_table;

  // Populate the table with the entries from the buffer
  for (int i = 0; i < load_table_size; i++) {
    load_table[i].offset = ((uint32_t *) buffer + 3 + (i * 3))[0];
    load_table[i].size   = ((uint32_t *) buffer + 3 + (i * 3))[1];
    load_table[i].load   = ((uint32_t *) buffer + 3 + (i * 3))[2];
  }

  // Allocate space for the executable
  *result = malloc(sizeof(Executable));
  if (!(*result)) {
    return exe_err_allocation;
  }

  size_t data_segment_size = size - 12 - (load_table_size * 12);
  uint8_t* input_data = buffer + 12 + (load_table_size * 12);

  // Allocate space for the data segment
  uint8_t* data_segment = malloc(data_segment_size);
  if (!data_segment) {
    return exe_err_allocation;
  }

  // Copy the data segment from the input buffer into the buffer
  // we just allocated
  memcpy(data_segment, input_data, data_segment_size);

  (*result)->header = header;
  (*result)->data = data_segment;
  (*result)->data_size = data_segment_size;

  return exe_err_success;
}

/*
 * Prints information about a given executable
 * */
void exe_print_info(Executable* exe) {
  printf("Entry address: 0x%08x\n", exe->header->entry_addr);
  printf("Load Table:\n");

  size_t size = exe->header->load_table_size;
  for (int i = 0; i < size; i++) {
    printf("0x%08x : %7d bytes : 0x%08x\n",
      exe->header->load_table[i].offset,
      exe->header->load_table[i].load,
      exe->header->load_table[i].load
    );
  }

  printf("\n");

  printf("Data size: %zu bytes\n", exe->data_size);

  return;
}

/*
 * Clean the resources used by an Executable struct
 * */
void exe_clean(Executable* exe) {
  if (exe == NULL) return;

  free(exe->header->load_table);
  free(exe->header);
  free(exe->data);
  free(exe);
  return;
}

/*
 * Returns a human-readable error message for a given error code
 * */
char* exe_err(ExecutableError errcode) {
  switch (errcode) {
    case exe_err_success:
      return "Success";
    case exe_err_too_small:
      return "Executable too small";
    case exe_err_invalid_magicnum:
      return "Invalid magic number";
    case exe_err_allocation:
      return "Allocation failure";
    default:
      return "Unknown error";
  }
}
