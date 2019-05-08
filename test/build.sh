#!/bin/sh

build_dir="bin/"

if [ ! -d $build_dir ]; then
    mkdir bin/
fi

src="create_default_scene.cpp"
# ext: stb libs, rapidjson
# include: config parser headers
inc="-I/../ext -I/../include/"
compile="g++ -g -Wall -std=c++11"
out="-o bin/main"

exec $compile -I/../ext/ -I/../include/ -c $src $out
