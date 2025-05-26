C_SRC := $(wildcard *.c)
ASM_SRC := $(wildcard *.S)
OBJ := $(C_SRC:.c=.o) $(ASM_SRC:.S=.o) $(wildcard ../*.o)
DEPS := $(C_SRC:.c=.d) $(ASM_SRC:.S=.d) $(wildcard ../*.d)

.PHONY: all clean

all: $(OBJ)

clean:
	rm -f *.o *.d

%.o: %.c
	$(GCC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $<

%.o: %.S
	$(GCC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $<

-include $(DEPS)
