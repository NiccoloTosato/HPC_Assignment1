SRC = matrix.c
CC = mpicc
CFLAGS = -Wall -Wextra
CLAGS_D = -Wall -Wextra -ggdb3 -DDEBUG
EXE = $(SRC:.c=.x)


info::
	@echo "NT makefile, available options:"
info::
	@echo "make default"
default:
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) -O3
info::
	@echo "make format"
format:
	clang-format -i $(SRC) -verbose
info::
	@echo "make print1"
print1::
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) -DPRINT1	
info::
	@echo "make print2"
print2::
	$(CC) $(SRC) -o $(EXE) $(CFLAGS) -DPRINT2	
info::
	@echo "make clean"
clean:
	rm $(EXE)
