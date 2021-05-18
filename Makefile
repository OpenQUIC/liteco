CC = cc
AR = ar
INCLUDE = -I ./include -I ./src/

OBJECTS = 
C_OBJS = 

C_OBJS += src/lc_addr.o
C_OBJS += src/lc_async.o
C_OBJS += src/lc_channel.o
C_OBJS += src/lc_coroutine.o
C_OBJS += src/lc_heap.o
C_OBJS += src/lc_rbt.o
C_OBJS += src/lc_runtime.o
C_OBJS += src/lc_timer.o
C_OBJS += src/lc_udp.o

OBJECTS += $(C_OBJS)

ASM_SRC := $(wildcard $(OBJECTS:o=s))
ASM_OBJ := $(ASM_SRC:s=o)
C_OBJ := $(filter-out $(ASM_OBJ),$(OBJECTS))

QUIET_CC = @echo '  ' CC $@;

.SUFFIXS:

all: $(C_OBJ)


$(C_OBJ): %.o: %.c
	$(QUIET_CC) $(CC) $(INCLUDE) -o $*.o -c $<
