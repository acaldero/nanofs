
all: createdisk compile run

createdisk:
	@echo "Creating disk..."
	dd if=/dev/zero of=disk.dat bs=1024 count=32
	@echo ""

compile:
	@echo "Compiling..."
	gcc -Wall -g -o block.o  -c block.c
	gcc -Wall -g -o nanofs.o -c nanofs.c
	gcc -Wall -g -o test.o   -c test.c
	gcc -Wall -g -o test test.o nanofs.o block.o
	@echo ""

run:
	@echo "Running..."
	./test
	@echo ""

view:
	@echo "Exploring raw disk.dat as char array..."
	od -A d -c disk.dat 

clean:
	@echo "Cleaning..."
	rm -fr test *.o test.dSYM

help:
	@echo ""
	@echo "make createdisk: create disk.dat"
	@echo "make compile:    compile files"
	@echo "make run:        run the test"
	@echo "make clean:      clean intermediated files"
	@echo ""

