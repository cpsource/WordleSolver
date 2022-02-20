
GIT_VERSION := "$(shell git describe --abbrev=8 --dirty --always --tags)"
BUILD_DATE  := "$(shell date)"

all: setup wos

setup: setup.c
	gcc -O2 -o setup setup.c

wos: config.h version.h wos.c
	gcc -DGIT_VERSION=\"$(GIT_VERSION)\" -DBUILD_DATE=\"$(BUILD_DATE)\" -Wall -Werror -O2 -o wos sbs.c

