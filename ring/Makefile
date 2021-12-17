SRC = ring.c
CC = mpicc
CFLAGS = -Wall -Wextra -lm
CFLAGS_D = -Wall -Wextra -g -lm -DDEBUG
EXE = $(SRC:.c=.x)


info::
	@echo "\nNT makefile, available options:\n"
info::
	@echo "make default: requested program with output on file\n"
default:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS)
info::
	@echo "make debug: additional info printed,timing and final messages\n"
debug:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS_D)
info::
	@echo "make debug2: like previous options plus additional info about virtual topology \n"
debug2:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS_D) -DDEBUG2

info::
	@echo "make csv: suitable output to create csv file with timings\n"
csv:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) -DCSV
info::
	@echo "make format: code formatter using clang\n"
format:
	clang-format -i $(SRC) -verbose 
info::
	@echo "make clean: rm ring.x \n"
clean:
	rm $(EXE)
