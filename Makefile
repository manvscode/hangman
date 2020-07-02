
#CFLAGS = -std=c11 -D_DEFAULT_SOURCE -O0 -g -I /usr/local/include -I extern/include/ -I extern/include/collections-1.0.0/ -I extern/include/xtd-1.0.0/
CFLAGS = -std=c11 -D_DEFAULT_SOURCE -O2 -I /usr/local/include -I extern/include/collections-1.0.0/ -I extern/include/xtd-1.0.0/
LDFLAGS = -lm extern/lib/libxtd.a extern/lib/libcollections.a -L /usr/local/lib -L extern/lib/ -L extern/libcollections/lib/
CWD = $(shell pwd)
BIN_NAME = hangman

SOURCES = src/main.c

all: extern/libxtd extern/libcollections bin/$(BIN_NAME)

bin/$(BIN_NAME): $(SOURCES:.c=.o)
	@mkdir -p bin
	@echo "Linking: $^"
	@$(CC) $(CFLAGS) -o bin/$(BIN_NAME) $^ $(LDFLAGS)
	@echo "Created $@"

src/%.o: src/%.c
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

#################################################
# Dependencies                                  #
#################################################
extern/libxtd:
	@mkdir -p extern/libxtd/
	@git clone https://bitbucket.org/manvscode/libxtd.git extern/libxtd/
	@cd extern/libxtd && autoreconf -i && ./configure --libdir=$(CWD)/extern/lib/ --includedir=$(CWD)/extern/include/ && make && make install

extern/libcollections:
	@mkdir -p extern/libcollections/
	@git clone https://bitbucket.org/manvscode/libcollections.git extern/libcollections/
	@cd extern/libcollections && autoreconf -i && ./configure --libdir=$(CWD)/extern/lib/ --includedir=$(CWD)/extern/include/ && make && make install

#################################################
# Cleaning                                      #
#################################################
clean_extern:
	@rm -rf extern

clean:
	@rm -rf src/*.o
	@rm -rf bin
