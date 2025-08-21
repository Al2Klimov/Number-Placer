# Build the Rust version by default
number-placer: src/main.rs Cargo.toml
	cargo build --release
	cp target/release/number-placer .

# Build the original C++ version
number-placer-cpp: number-placer.cpp
	g++ -std=c++11 -pedantic-errors -fno-rtti -O3 -Wfatal-errors -Wall -Wextra \
		-static \
		-o number-placer-cpp number-placer.cpp

# Build both versions for comparison
all: number-placer number-placer-cpp

test: number-placer number-placer-cpp
	@echo "Testing both versions with same input..."
	@echo "123456789000000000000000000000000000000000000000000000000000000000000000000000000" > test-input.txt
	@echo "=== Rust version ==="
	@cat test-input.txt | ./number-placer
	@echo "=== C++ version ==="
	@cat test-input.txt | ./number-placer-cpp
	@rm -f test-input.txt

clean:
	rm -f number-placer number-placer-cpp
	cargo clean

.PHONY: all test clean
