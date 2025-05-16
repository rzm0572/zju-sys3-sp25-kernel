LS_FUNCTION   = $(filter $(1), $(shell ls))

SRC_C = $(call LS_FUNCTION,%.c)
SRC_S = $(call LS_FUNCTION,%.S)

INCLUDE_FLAG = -I ../include -I ../../../include
OBJ = $(patsubst %.c,%.o,$(SRC_C)) $(patsubst %.S,%.o,$(SRC_S))

all: $(OBJ)

%.o:%.c
	$(GCC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $<

%.o:%.S
	$(GCC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c $<

clean:
	rm -rf *.o *.d
