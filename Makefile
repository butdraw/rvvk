# CROSS_PREFIX:=~/riscv/bin/riscv64-unknown-linux-gnu
CROSS_PREFIX:=~/riscv/bin/riscv64-unknown-elf

CC=cc
LD=gcc
CFLAGS:=-I./include -Wall -Werror -g -O0 -std=c99
LDFLAGS:=-static

TESTS_CFLAGS:=-Wall -Werror -g -O0 -std=c99
TESTS_LDFLAGS:=-static

RVCC=$(CROSS_PREFIX)-gcc

DIRS:=. arch
DIRS+=arch/riscv
SRCS:=$(addsuffix /*.c,$(DIRS))

OBJDIR:=build
OBJS:=$(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(wildcard $(SRCS))))

PROG:=rvvk
TESTS:=test_hello

.PHONY: all rvkk

all: $(PROG) $(TESTS)

rvvk: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR):
	mkdir -p $(addprefix $(OBJDIR)/, $(DIRS))

%: tests/%.c
	$(RVCC) $(TESTS_LDFLAGS) $(TESTS_CFLAGS) -z separate-code -o $@ $<

debug:
	@echo "CC            = $(CC)"
	@echo "LD            = $(LD)"
	@echo "CFLAGS        = $(CFLAGS)"
	@echo "LDFLAGS       = $(LDFLAGS)"
	@echo "RVCC          = $(RVCC)"
	@echo "DIRS          = $(DIRS)"
	@echo "SRCS          = $(SRCS)"
	@echo "OBJDIR        = $(OBJDIR)"
	@echo "OBJS          = $(OBJS)"
	@echo "PROG          = $(PROG)"
	@echo "TESTS_CFLAGS  = $(TESTS_CFLAGS)"
	@echo "TESTS_LDFLAGS = $(TESTS_LDFLAGS)"
	@echo "TESTS         = $(TESTS)"

clean:
	rm -rf $(OBJDIR) $(PROG) $(TESTS)
