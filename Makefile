LIBS     ?= -lraylib -lm -lcjson
FLAGS    ?= -Wall -Wextra
CC       ?= gcc

BUILDDIR := build
BINDIR   := $(BUILDDIR)/bin
OBJDIR   := $(BUILDDIR)/obj
TARGET   := $(BINDIR)/fose

SRCS     := $(shell find . -type f -name '*.c' | sort)
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
	@printf '%b\n' '$(GREEN)DONE.$(RESET)'

$(OBJDIR)/%.o: %.c
	@printf '%b\n' '$(CYAN)$(BOLD)==>$(RESET) Compiling $< -> $@'
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(DEPS)

# run: all
# 	@printf '%b\n' '$(CYAN)$(BOLD)==>$(RESET) Running $(TARGET)\n'
# 	@./$(TARGET)

clean:
	@printf '%b\n' '$(RED)$(BOLD)==>$(RESET) Cleaning build files...'
	@rm -rf $(BUILDDIR)
	@printf '%b\n' '$(GREEN)DONE.$(RESET)'
