override BIN := example

override SRC_example := example.c letopt.c
override LIB_example := -lgmp

override SRC := $(foreach b,$(BIN),$(SRC_$b))
override OBJ := $(SRC:%=%.o)
override DEP := $(SRC:%=%.d)

override ARCHFLAGS := -march=native -mtune=native
override WARNFLAGS := -Wall -Wextra -Wpedantic
override BASEFLAGS := $(WARNFLAGS) -pipe -flto -O3 $(ARCHFLAGS)

CC       := gcc
CFLAGS   := $(BASEFLAGS) -std=gnu2x
CPPFLAGS := -DNDEBUG=1

all: $(BIN)

$(foreach b,$(BIN),$(eval $b: $$(SRC_$b:%=%.o); \
	$$(CC) $$(CPPFLAGS) $$(CFLAGS) -o $$@ $$^ $$(LIB_$$@)))

%.c.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c -MMD $<

purge: override purge_FILES = $(wildcard $(DEP))
purge: | clean; $(if $(purge_FILES),$(RM) $(purge_FILES),@:)

clean: override clean_FILES = $(wildcard $(BIN) $(OBJ))
clean:; $(if $(clean_FILES),$(RM) $(clean_FILES),@:)

.PHONY: all clean purge

-include $(DEP)
