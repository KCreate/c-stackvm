#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "vm.h"
#include "exe.h"

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

/*
 * Clean the resources used by a vm struct
 * */
void vm_clean(VM* vm) {
  if (vm == NULL) return;

  free(vm->memory);
  free(vm->regs);
  return;
}
