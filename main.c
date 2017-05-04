#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h>
#include "machine.h"

/*
 * Allocate the memory for VM struct
 * Returns false if allocation failed
 * */
bool vm_create(VM** vm) {
  if (vm == NULL) {
    return false;
  }

  VM* vm_ptr = calloc(1, sizeof(VM));
  byte* memory = calloc(VM_MEMORYSIZE, sizeof(byte));
  qword* regs = calloc(VM_REGCOUNT, sizeof(qword));

  if (vm_ptr == NULL || memory == NULL || regs == NULL) {
    return false;
  }

  vm_ptr->memory = memory;
  vm_ptr->regs = regs;
  vm_ptr->running = true;
  vm_ptr->exit_code = 0;

  *vm = vm_ptr;
  return true;
}

int main(int argc, char** argv) {
  VM* vm; // virtual machine
  FILE* fp; // input file pointer

  // Check for the filename
  if (argc <= 1) {
    fprintf(stderr, "Missing filename\n");
    return 1;
  }

  fp = fopen(argv[1], "r");

  if (fp == NULL) {
    fprintf(stderr, "Could not open file: %s\n", argv[1]);
    return 1;
  }

  struct stat inputStat;
  if (fstat(fileno(fp), &inputStat) < 0) {
    fprintf(stderr, "Could not stat file: %s\n", argv[1]);
    return 1;
  }

  char* buffer = malloc(inputStat.st_size);

  if (buffer == NULL) {
    fprintf(stderr, "Could not allocate space for executable\n");
    return 1;
  }

  fread(buffer, inputStat.st_size, 1, fp);

  if (!vm_create(&vm)) {
    fprintf(stderr, "Could not initialize vm\n");
    return 1;
  }

  fwrite(buffer, inputStat.st_size, 1, stdout);

  Executable* exe = exe_create(buffer, inputStat.st_size);

  vm_clean(vm);
  exe_clean(exe);
  fclose(fp);

  return 0;
}

/*
 * Parse an executable from buffer
 * */
Executable* exe_create(char* buffer, size_t size) {
  return NULL;
};

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
