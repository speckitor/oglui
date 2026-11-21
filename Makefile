all: bin

bin:
	mkdir build
	gcc -o build/bin src/* ./thirdparty/glad/src/gl.c -I./thirdparty/glad/include/ -lglfw
