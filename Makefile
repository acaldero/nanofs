
all: createdisk compile run
	@echo "Done."

createdisk:
	@echo "Creating disk..."
	dd if=/dev/zero of=tmpfile.dat bs=1024 count=32

compile:
	@echo "Compiling..."
	gcc -o nanofs -Wall -g nanofs.c

run:
	@echo "Running..."
	./nanofs

view:
	@echo "Exploring raw tmpfile.dat as char array..."
	od -c tmpfile.dat 

clean:
	@echo "Cleaning..."
	rm -fr nanofs nanofs.o nanofs.dSYM

help:
	@echo ""
	@echo "make createdisk: create tmpfile.dat"
	@echo "make compile:    compile files"
	@echo "make run:        run the nanofs"
	@echo "make clean:      clean intermediated files"
	@echo ""

