
build_directory="bin/"

if [ ! -d "bin/" ]; then
  mkdir bin/
fi

src="src/main.cpp"
inc="-I./inc"
compile="g++ -std=c++11"
out="-o bin/client"

exec $compile $inc $src $out