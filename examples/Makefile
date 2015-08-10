sourcedir = src/compiler
includedir = src/compiler/includes

prog: ./build/windows ./build/objects ./build/fib ./build/fileio ./build/helloworld ./build/higherorder ./build/lists ./build/pythag ./build/strings ./build/windows

./build/fib: ./src/fib.vpr
	viper ./src/fib.vpr -o ./build/fib

./build/fileio: ./src/fileio.vpr
	viper ./src/fileio.vpr -o ./build/fileio

./build/helloworld: ./src/helloworld.vpr
	viper ./src/helloworld.vpr -o ./build/helloworld

./build/higherorder: ./src/higherorder.vpr
	viper ./src/higherorder.vpr -o ./build/higherorder

./build/lists: ./src/lists.vpr
	viper ./src/lists.vpr -o ./build/lists

./build/pythag: ./src/pythag.vpr
	viper ./src/pythag.vpr -o ./build/pythag

./build/strings: ./src/strings.vpr
	viper ./src/strings.vpr -o ./build/strings

./build/windows: ./src/windows.vpr
	viper ./src/windows.vpr -o ./build/windows

./build/objects: ./src/objects.vpr
	viper ./src/objects.vpr -o ./build/objects