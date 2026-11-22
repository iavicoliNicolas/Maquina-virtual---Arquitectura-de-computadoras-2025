# Build the VMX25 virtual machine.
# Flags match the Code::Blocks Debug target in mv/mv.cbp (gcc -Wall -g).

# Pin gcc, the compiler selected in mv/mv.cbp. GNU make's default CC is
# `cc`, and Clang rejects the implicit `gets` declaration in funciones.c.
CC      := gcc
CFLAGS  := -Wall -g -Imv/include

SRCS := mv/Source/main.c \
        mv/Source/mv.c \
        mv/Source/funciones.c \
        mv/Source/operando.c \
        mv/Source/disassembler.c \
        mv/Source/paramsegment.c

HDRS := mv/include/mv.h \
        mv/include/funciones.h \
        mv/include/operando.h \
        mv/include/disassembler.h \
        mv/include/paramsegment.h

TARGET := build/mv

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRCS)

clean:
	rm -rf build
