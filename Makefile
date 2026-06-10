CC = gcc
CFLAGS = -std=c99 -Wall -O3 -fvisibility=hidden
NAME = libkipcorn.a

KIPCORN_WL ?= 0
KIPCORN_XCB ?= 0
KIPCORN_OPENGL ?= 0

INC_DIRS = include external
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

PKGS =

ifeq ($(KIPCORN_WL), 1)
	PKGS += wayland-client xkbcommon
	CFLAGS += -DKIPCORN_WL
endif

ifeq ($(KIPCORN_XCB), 1)
	PKGS += xcb
	CFLAGS += -DKIPCORN_XCB
endif

ifeq ($(KIPCORN_XLIB), 1)
	PKGS += x11
	CFLAGS += -DKIPCORN_XLIB
endif

PKG_CFLAGS = $(shell pkg-config --cflags $(PKGS))

ALL_CFLAGS = $(CFLAGS) $(PKG_CFLAGS) $(INC_FLAGS)

SRCS = $(shell find src external -name "*.c")
OBJS = $(SRCS:%.c=build/%.o)
DEPS = $(OBJS:.o=.d)

all: $(NAME)

-include $(DEPS)

$(NAME): $(OBJS)
	@echo "Archiving $@..."
	@ar rcs $@ $(OBJS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "CC $<"
	@$(CC) $(ALL_CFLAGS) -MMD -MP -c $< -o $@

clean:
	@echo "Cleaning..."
	rm -rf build $(NAME)

print-libs:
	@echo $(abspath libkipcorn.a)
	@pkg-config --libs $(PKGS)

print-incs:
	@for dir in $(INC_FLAGS); do \
	    case $$dir in \
	        -I*) echo -n "-I$(abspath $${dir#-I}) " ;; \
	        *) echo -n "$$dir " ;; \
	    esac; \
	done
	@echo

.PHONY: all clean print-libs print-incs