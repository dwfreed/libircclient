LIBRARY_NAME = libircclient

# All of the C files we'll need go here, but with a .o extension
OBJS = errors.o libircclient.o

LIBS = glib-2.0 gthread-2.0

CC = gcc -ggdb -Wall -Werror -Wextra
CFLAGS = -fPIC $(shell pkg-config --cflags ${LIBS})
LDFLAGS = $(shell pkg-config --libs ${LIBS})

all: ${LIBRARY_NAME}.so ${LIBRARY_NAME}.a

${LIBRARY_NAME}.so: ${OBJS}
	@echo "Building $@"
	@${CC} -shared -nostartfiles -nostdlib -fPIC ${OBJS} ${LDFLAGS} -o $@

${LIBRARY_NAME}.a: ${OBJS}
	@echo "Building $@"
	@$(AR) rc ${LIBRARY_NAME}.a ${OBJS}
	@[[ -z "$(RANLIB)" ]] || $(RANLIB) ${LIBRARY_NAME}.a

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
