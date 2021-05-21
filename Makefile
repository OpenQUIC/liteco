# Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
#
# Distributed under the MIT software license, see the accompanying
# file LICENSE or https://www.opensource.org/licenses/mit-license.php .

CC = cc
AR = ar
RM = rm -f
INCLUDE = -I ./include -I ./src/
ARFLAGS = rcs

LIB_FILE = libliteco.a

OBJECTS = 
C_OBJS = 
ASM_OBJS = 
ARCH = $(shell uname -m)
FORMAT = 
PLATFORM = 

ifeq ($(shell uname), Linux)
	FORMAT = elf
	PLATFORM = linux
else ifeq ($(shell uname), Darwin)
	FORMAT = macho
	PLATFORM = darwin
endif

C_OBJS += src/lc_addr.o
C_OBJS += src/lc_async.o
C_OBJS += src/lc_channel.o
C_OBJS += src/lc_coroutine.o
C_OBJS += src/lc_heap.o
C_OBJS += src/lc_rbt.o
C_OBJS += src/lc_runtime.o
C_OBJS += src/lc_timer.o
C_OBJS += src/lc_udp.o
C_OBJS += src/platform/$(PLATFORM)/internal.o
C_OBJS += src/platform/$(PLATFORM)/lc_eloop.o
C_OBJS += src/platform/$(PLATFORM)/lc_io.o

ARM_OBJS += src/arch/$(ARCH)/$(FORMAT)/cas.o
ARM_OBJS += src/arch/$(ARCH)/$(FORMAT)/init.o
ARM_OBJS += src/arch/$(ARCH)/$(FORMAT)/swap.o
ARM_OBJS += src/arch/$(ARCH)/$(FORMAT)/yield.o

OBJECTS += $(C_OBJS)
OBJECTS += $(ARM_OBJS)

QUIET_CC = @echo '  ' CC $@;
QUIET_RM_OBJECT = @echo '  ' RM $(@:%.o_del=%.o);
QUIET_AR = @echo '  ' AR $@;

.SUFFIXS:


$(LIB_FILE): $(OBJECTS)
	$(QUIET_AR) $(RM) $@ && $(AR) $(ARFLAGS) $@ $^

$(C_OBJS): %.o: %.c
	$(QUIET_CC) $(CC) $(INCLUDE) -o $*.o -c $<

$(ARM_OBJS): %.o: %.s
	$(QUIET_CC) $(CC) -o $*.o -c $<

clean: $(OBJECTS:%.o=%.o_del)
	$(RM) $(LIB_FILE)


$(OBJECTS:%.o=%.o_del): 
	$(QUIET_RM_OBJECT)
	if [ -e $(@:%.o_del=%.o) ]; then $(RM) $(@:%.o_del=%.o); fi
