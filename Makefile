LIBS     ?= -lraylib -lm -lcjson
FLAGS    ?= -Wall -Wextra
CC       ?= gcc

BUILDDIR := build
BINDIR   := $(BUILDDIR)/bin
OBJDIR   := $(BUILDDIR)/obj
TARGET   := $(BINDIR)/fose

SRCS     := $(shell find src -type f -name '*.c' | sort)
UTLS     := util/ft.c
OBJS     := $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)

BOLD     := \033[1m
GREEN    := \033[0;32m
CYAN     := \033[0;36m
RED      := \033[0;31m
RESET    := \033[0m


.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@printf '%b\n' '$(CYAN)$(BOLD)==>$(RESET) Linking fose...'
	@mkdir -p $(BINDIR)
	@$(CC) $(FLAGS) $(OBJS) -o $@ $(LIBS)
	@if [[ -L fose ]]; then echo "fose link already exists."; else ln -s $(BINDIR)/fose fose; fi
	@printf '%b\n' '$(CYAN)$(BOLD)==>$(RESET) Building $(UTLS)'
	@$(CC) -o $(BINDIR)/ft $(UTLS) -lm
	@if [[ -L ft ]]; then echo "ft link already exists."; else ln -s $(BINDIR)/ft ft; fi
	@printf '%b\n' '$(GREEN)DONE.$(RESET)'

$(OBJDIR)/%.o: %.c
	@printf '%b\n' '$(CYAN)$(BOLD)==>$(RESET) Compiling $< -> $@'
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(DEPS)

clean:
	@printf '%b\n' '$(RED)$(BOLD)==>$(RESET) Cleaning build files...'
	@rm -rf $(BUILDDIR) 
	@if [[ -L fose ]]; then rm ./fose; fi
	@if [[ -L ft ]]; then rm ./ft; fi
	@printf '%b\n' '$(GREEN)DONE.$(RESET)'
