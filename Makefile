CXXFLAGS = -march=native -std=c++20

fakeblue:

blue.pbm: fakeblue
	./fakeblue

blue.png: blue.pbm
	convert blue.pbm blue.png
