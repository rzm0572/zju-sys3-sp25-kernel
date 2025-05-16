C_SRC   := $(wildcard *.c)
ASM_SRC := $(wildcard *.S)
OBJ := $(C_SRC:.c=.o) $(ASM_SRC:.S=.o)

.PHONY: all clean

all: $(OBJ)

clean:
	rm -f *.o

%.o: %.c
	$(GCC) $(CFLAG) -c $< -o $@

%.o: %.S
	$(GCC) $(CFLAG) -c $< -o $@
