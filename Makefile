CXXFLAGS = -march=native -O3 -std=c++20 -ggdb3

blue.png: blue.pbm
	convert blue.pbm blue.png

blue.pbm: fakeblue
	./fakeblue

fakeblue: fakeblue.cpp

clean:
	rm -f fakeblue blue.*

.PHONY: clean
