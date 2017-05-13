CC=clang
OPT=-O0
CFLAGS=-g -fdata-sections -ffunction-sections $(OPT)
VM_OBJS=obj/main.o obj/vm.o obj/exe.o

VM_CYCLES=8

assemble: vm test.asm
	stackvm build test.asm -s -o test.bc
	bin/vm test.bc $(VM_CYCLES)

vm: $(VM_OBJS)
	$(CC) $(CFLAGS) $(VM_OBJS) -dead_strip -o bin/vm

clean:
	rm -f .DS_Store
	rm -rf bin
	rm -rf obj
	mkdir bin obj

setup:
	mkdir bin obj

# $@ is the name of the file that's being generated
# $< is the name of the first prerequisite (in this case the source file)
obj/%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
