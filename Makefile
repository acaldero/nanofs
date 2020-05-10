
all: createdisk compile run

createdisk:
	@echo "Creating disk..."
	dd if=/dev/zero of=disk.dat bs=1024 count=32
	@echo ""

compile:
	@echo "Compiling..."
	gcc -o nanofs -Wall -g nanofs.c
	@echo ""

run:
	@echo "Running..."
	./nanofs
	@echo ""

view:
	@echo "Exploring raw disk.dat as char array..."
	od -c disk.dat 

clean:
	@echo "Cleaning..."
	rm -fr nanofs nanofs.o nanofs.dSYM

help:
	@echo ""
	@echo "make createdisk: create disk.dat"
	@echo "make compile:    compile files"
	@echo "make run:        run the nanofs"
	@echo "make clean:      clean intermediated files"
	@echo ""

