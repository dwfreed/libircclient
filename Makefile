LIBRARY_NAME = libircclient

# All of the C files we'll need go here, but with a .o extension
OBJS = libircclient.o

LIBS = glib-2.0 gthread-2.0

CC = gcc
CFLAGS = -fPIC -march=native -Wall -Werror -Wextra -D_GNU_SOURCE $(shell pkg-config --cflags ${LIBS})
LDFLAGS = -fPIC $(shell pkg-config --libs ${LIBS})

DEBUG = 0

ifeq (${DEBUG},1)
	CFLAGS += -ggdb3 -D_FORTIFY_SOURCE=2
else
	CFLAGS += -O3
	LDFLAGS += -O3
endif

all: ${LIBRARY_NAME}.so ${LIBRARY_NAME}.a

${LIBRARY_NAME}.so: ${OBJS}
	@echo "Building $@"
	@${CC} -shared -nostartfiles -nostdlib ${OBJS} ${LDFLAGS} -o $@

${LIBRARY_NAME}.a: ${OBJS}
	@echo "Building $@"
	@$(AR) rc ${LIBRARY_NAME}.a ${OBJS}
	@[ -z "$(RANLIB)" ] || $(RANLIB) ${LIBRARY_NAME}.a

global.h: libircclient.h

libircclient.h: libircclient_errors.h libircclient_events.h libircclient_options.h libircclient_params.h libircclient_rfcnumerics.h libircclient_session.h

%.o: %.c Makefile global.h
	@echo "Compiling $<"
	@${CC} ${CFLAGS} -c $< -o $@

%.i: %.c Makefile global.h
	@echo "Prepocessing $<"
	@${CC} ${CFLAGS} -E $< -o $@

clean:
	@echo "Cleaning ${LIBRARY_NAME}"
	@rm -f ${LIBRARY_NAME}.so ${LIBRARY_NAME}.a ${OBJS}

.PHONY: all clean
