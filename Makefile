.PHONY: all examples clean

all: tarpit

tarpit: obj/compiler.o obj/bytes.o obj/stack.o obj/barf.o
	gcc -Wall -Werror -o tarpit obj/compiler.o obj/bytes.o obj/stack.o obj/barf.o

obj/compiler.o: src/compiler.c src/config.h src/bytes.h src/stack.h
	@mkdir -p obj
	gcc -Wall -Werror -c src/compiler.c -o obj/compiler.o

obj/bytes.o: src/bytes.c src/config.h src/bytes.h src/barf.h
	@mkdir -p obj
	gcc -Wall -Werror -c src/bytes.c -o obj/bytes.o

obj/stack.o: src/stack.c src/config.h src/stack.h
	@mkdir -p obj
	gcc -Wall -Werror -c src/stack.c -o obj/stack.o

obj/barf.o: src/barf.c src/barf.h
	@mkdir -p obj
	gcc -Wall -Werror -c src/barf.c -o obj/barf.o

examples: helloworld rot13

helloworld: examples/helloworld.bf tarpit
	./tarpit examples/helloworld.bf helloworld

rot13: examples/rot13.bf tarpit
	./tarpit examples/rot13.bf rot13

clean:
	rm tarpit obj/* helloworld rot13
