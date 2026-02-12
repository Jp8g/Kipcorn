CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O3 -fPIC
LIB_NAME = libkipcorn.a

INC_FLAGS = -Iinclude -Iexternal

PKGS = wayland-client egl wayland-egl xkbcommon
PKG_CFLAGS = $(shell pkg-config --cflags $(PKGS))
PKG_LIBS   = $(shell pkg-config --libs $(PKGS))

ALL_CFLAGS = $(CFLAGS) $(INC_FLAGS) $(PKG_CFLAGS)

SRCS = $(shell find src external -name "*.c")

OBJS = $(SRCS:%.c=build/%.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	@echo "Archiving $@..."
	@ar rcs $@ $(OBJS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -rf build $(LIB_NAME)

print-pkgs:
	@echo $(PKGS)

.PHONY: all clean print-pkgs