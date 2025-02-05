CXXFLAGS = -march=native -std=c++20

blue.png: blue.pbm
	convert blue.pbm blue.png

blue.pbm: fakeblue
	./fakeblue

fakeblue:

clean:
	rm -f fakeblue blue.*

.PHONY: clean
