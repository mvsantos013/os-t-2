all: build

build:
	gcc -o main.out src/main.c -Wall

start:
	make build && ./main.out