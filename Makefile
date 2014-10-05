number-placer: number-placer.cpp
	g++ -std=c++11 -pedantic-errors -fno-rtti -O3 -Wfatal-errors -Wall -Wextra \
		-o number-placer number-placer.cpp

clean:
	rm -f number-placer
