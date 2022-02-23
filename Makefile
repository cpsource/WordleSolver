
GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"
BUILD_DATE  := "$(shell date)"

all: setup wos

setup: setup.c
	gcc -O2 -o setup setup.c

freq.o: freq.c freq.h
	gcc -O2 -c freq.c -Wall -Werror -Wno-char-subscripts

best_guess.o: best_guess.c best_guess.h
	gcc -O2 -c best_guess.c -Wall -Werror -Wno-char-subscripts

nytwords.o: nytwords.c nytwords.h
	gcc -O2 -c nytwords.c -Wall -Werror -Wno-char-subscripts

wos: config.h version.h wos.c freq.o best_guess.o
	gcc -g -DGIT_VERSION=\"$(GIT_VERSION)\" -DBUILD_DATE=\"$(BUILD_DATE)\" -Wall -Werror -o wos wos.c -Wno-char-subscripts freq.o nytwords.o best_guess.o

