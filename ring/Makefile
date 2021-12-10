SRC = ring.c
CC = mpicc
CFLAGS = -Wall -Wextra -lm
CFLAGS_D = -Wall -Wextra -g -lm -DDEBUG
EXE = $(SRC:.c=.x)


info::
	@echo "NT makefile, available options:"
info::
	@echo "make default"
default:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS)
info::
	@echo "make debug: additional info printed"
debug:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS_D)
info::
	@echo "make debug2: additional info about virtual topology"
debug2:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS_D) -DDEBUG2

info::
	@echo "make csv: suitable output to create csv file"
csv:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) -DCSV
info::
	@echo "make format"
format:
	clang-format -i $(SRC) -verbose 
info::
	@echo "make clean"
clean:
	rm $(EXE)
