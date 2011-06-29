LIBRARY_NAME = libircclient

# All of the C files we'll need go here, but with a .o extension
OBJS = libircclient.o

LIBS = glib-2.0 gthread-2.0

CC = gcc
CCFLAGS = -fPIC
CFLAGS = -Wall -Werror -Wextra -D_GNU_SOURCE $(shell pkg-config --cflags ${LIBS})
LDFLAGS = $(shell pkg-config --libs ${LIBS})

DEBUG = 0

ifeq (${DEBUG},1)
	CFLAGS += -ggdb3 -D_FORTIFY_SOURCE=2
	CCFLAGS += -pg -fprofile-arcs -ftest-coverage -fprofile-generate
	DEBUG_OBJS = test test-server
else
	CCFLAGS += -O3 -fprofile-use -fprofile-correction
endif

all: ${LIBRARY_NAME}.so ${LIBRARY_NAME}.a ${DEBUG_OBJS}

${LIBRARY_NAME}.so: ${OBJS}
	@echo "Building $@"
	@${CC} ${CCFLAGS} -shared -nostartfiles -nostdlib ${OBJS} ${LDFLAGS} -o $@

${LIBRARY_NAME}.a: ${OBJS}
	@echo "Building $@"
	@$(AR) rc ${LIBRARY_NAME}.a ${OBJS}
	@[[ -z "$(RANLIB)" ]] || $(RANLIB) ${LIBRARY_NAME}.a

test: test.o test-server ${LIBRARY_NAME}.so
	@echo "Building $@"
	@${CC} ${CCFLAGS} -o $@ $< -Wl,-rpath,${CURDIR} -L${CURDIR} -lircclient ${LDFLAGS}

test-server: test-server.o
	@echo "Building $@"
	@${CC} ${CCFLAGS} -o $@ $< ${LDFLAGS}

global.h: libircclient.h

libircclient.h: libircclient_errors.h libircclient_events.h libircclient_options.h libircclient_params.h libircclient_rfcnumerics.h libircclient_session.h

%.o: %.c Makefile global.h
	@echo "Compiling $<"
	@${CC} ${CCFLAGS} ${CFLAGS} -c $< -o $@

%.i: %.c Makefile global.h
	@echo "Prepocessing $<"
	@${CC} ${CCFLAGS} ${CFLAGS} -E $< -o $@

clean:
	@echo "Cleaning ${LIBRARY_NAME}"
	@rm -f ${LIBRARY_NAME}.so ${LIBRARY_NAME}.a ${OBJS} test test_server test.o test_server.o

check: all
	@echo "Checking ${LIBRARY_NAME}"
	@./test-server
	@MALLOC_CHECK_=1 G_SLICE="all" G_DEBUG="gc-friendly" valgrind --leak-check=full --track-fds=yes --show-reachable=yes --track-origins=yes ./test

.PHONY: all clean check
