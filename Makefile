CC := gcc -std=c99
ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

sourcedir := src/compiler
includedir := src/compiler/includes

includes := $(includedir)/include.h $(includedir)/ast.h $(includedir)/bc.h
source := $(sourcedir)/main.c $(sourcedir)/typecheck.c $(sourcedir)/cgen.c $(sourcedir)/compile.c $(sourcedir)/astgen.c $(sourcedir)/readinstr.c $(sourcedir)/debug.c $(sourcedir)/readexpr.c 

prog: ./build/viper

./build/viper: $(includes) $(source)
	$(CC) $(source) -o ./build/viper