objects = 	main.o my_print.o
main	:	$(objects)
	g++ -o main $(objects)
main.o	:	main.cpp
	g++ -c -o main.o main.cpp
my_print.o :	my_print.asm
	nasm -f elf -o my_print.o my_print.asm
.PHONY	:	clean
clean	:
	rm main $(objects)
