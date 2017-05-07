#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "vm.h"
#include "exe.h"

int main(int argc, char** argv) {

  // Check for the filename
  if (argc <= 1) {
    fprintf(stderr, "Missing filename\n");
    return 1;
  }

  FILE* fp;
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

  uint8_t* buffer = malloc(inputStat.st_size);

  if (buffer == NULL) {
    fprintf(stderr, "Could not allocate space for executable\n");
    return 1;
  }

  fread(buffer, inputStat.st_size, 1, fp);

  Executable* exe;
  ExecutableError err = exe_create(&exe, buffer, inputStat.st_size);

  if (err != exe_err_success) {
    fprintf(stderr, "Could not parse executable: %s\n", exe_err(err));
    return 1;
  }

  exe_print_info(exe);

  VM* vm;

  VMError create_result = vm_create(&vm);
  if (create_result != vm_err_regular_exit) {
    fprintf(stderr, "Could not initialize vm\n");
    fprintf(stderr, "Reason: %s\n", vm_err(create_result));
    return 1;
  }

  VMError flash_result = vm_flash(vm, exe);
  if (flash_result != vm_err_regular_exit) {
    fprintf(stderr, "Could not load executable\n");
    fprintf(stderr, "Reason: %s\n", vm_err(flash_result));
  }

  for (int i = 0; i < 15; i++) {
    vm_cycle(vm);
  }

  for (int i = 0; i < 10; i++) {
    printf("reg%d: %08llu\n", i, vm->regs[i]);
  }

  vm_clean(vm);
  exe_clean(exe);
  fclose(fp);

  return 0;
}
