FLAGS=-std=c++11 -stdlib=libc++
SRC=$(wildcard source/*.cpp) $(wildcard source/**/*.cpp)

all: $(SRC)
	g++ $(FLAGS) -Wall $^ -o MySP

test:
	./MySP

.PHONY: all clean test
