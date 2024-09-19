CFLAGS = -Wall -Wextra -g -Os
CFLAGS += \
	  -ffreestanding \
	  -fno-builtin \
	  -fno-pie \
	  -fno-stack-protector \

CPPFLAGS = \
	   -Iinclude \

LDFLAGS = \
	  -no-pie \
	  -nostartfiles \
	  -nostdlib \
	  -Wl,-z,norelro \

LDSO_OBJS = \
	    ldso/ldso_start.o \
	    \
	    ldso/ldso.o \
	    $(LIBC_STDIO_OBJS) \
	    $(LIBC_STRING_OBJS) \
	    $(LIBC_UNISTD_OBJS) \
	    libc/malloc.o \


LIBC_BASE_OBJS = \
	    libc/libc_start_main.o \

LIBC_STDIO_OBJS = \
	    libc/printf.o \
	    libc/stdio.o \

LIBC_UNISTD_OBJS = \
	    libc/unistd.o\

LIBC_STRING_OBJS = \
	    libc/string.o \

LIBC_OBJS = \
	    $(LIBC_BASE_OBJS) \
	    $(LIBC_STDIO_OBJS) \
	    $(LIBC_STRING_OBJS) \
	    $(LIBC_UNISTD_OBJS) \
	    libc/malloc.o \


USELESS_OBJS = \
	    libc/useless.o \

TEST_LIBS = \
	    libuseless.so \
	    libc.so \
	    libunistd.so \
	    libstring.so \
	    libc2.so

TESTS = \
	test-standalone \
	test-onelib \
	test-libs \

MALLOC_CPPFLAGS = -include include/malloc-internal.h

TARGETS = ld.so


all: $(TARGETS) $(TEST_LIBS) $(TESTS)

$(TESTS):
	$(LINK.o) $^ $(LDLIBS) -o $@

$(TESTS): LDFLAGS += -Wl,--dynamic-linker=./ld.so -Wl,-rpath-link=.

test-standalone: LDLIBS = -L. -luseless
test-standalone: libc/crt0.o tests/test-standalone.o $(LIBC_OBJS)

test-onelib: LDLIBS = -L. -lc
test-onelib: libc/crt0.o tests/test-standalone.o

test-libs: LDLIBS = -L. -lc2
test-libs: libc/crt0.o tests/test-standalone.o

$(TEST_LIBS): CFLAGS += -fPIC

libc.so: $(LIBC_OBJS)

libc2.so: LDLIBS += -L. -lunistd -lstring
libc2.so: $(LIBC_BASE_OBJS) $(LIBC_STDIO_OBJS)

libunistd.so: $(LIBC_UNISTD_OBJS)
libstring.so: $(LIBC_STRING_OBJS)
libuseless.so: $(USELESS_OBJS)


ld.so: CFLAGS += -fPIC
ld.so: LDFLAGS += -Wl,--version-script,ldso/exported-symbols.map -Wl,-soname,ld.so
ld.so: $(LDSO_OBJS)

libc/printf.o: CPPFLAGS += -Iinclude/printf
libc/malloc.o: CPPFLAGS += $(MALLOC_CPPFLAGS)

%.so:
	$(LINK.o) -shared $^ $(LDLIBS) -o $@
