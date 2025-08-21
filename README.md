# Number-Placer

Al Klimov's Number Placer - A Sudoku solver written in Rust (rewritten from C++).

## Description

This program solves Sudoku puzzles, including:
- Standard Sudoku of configurable size (default 9x9)
- X-Sudoku (with diagonal constraints)

## Building

### Rust Version (Default)
```bash
make                    # Build optimized Rust version
```

### C++ Version (Original)
```bash
make number-placer-cpp  # Build original C++ version
```

### Both Versions
```bash
make all               # Build both versions
make test              # Test both versions with same input
```

## Usage

```bash
./number-placer [width] [height] [X]
```

- `width`: Block width (default: 3)
- `height`: Block height (default: 3)  
- `X`: Enable X-Sudoku mode (diagonal constraints)

### Examples

```bash
# Standard 9x9 Sudoku
echo "123456789000000000000000000000000000000000000000000000000000000000000000000000000" | ./number-placer

# 4x4 Sudoku
echo "1000000000000000" | ./number-placer 2 2

# X-Sudoku (with diagonals)
echo "123456789000000000000000000000000000000000000000000000000000000000000000000000000" | ./number-placer X
```

## Input Format

Input should be a string of numbers representing the puzzle:
- Length must match the total number of cells
- Each number uses a fixed width (enough digits for the maximum value)
- 0 represents empty cells
- Numbers 1-N represent filled cells

## Output Format

- Solved puzzle in the same format as input
- All zeros if the puzzle is unsolvable

## Implementation Notes

The Rust version is a faithful port of the original C++ implementation:
- Uses the same solving algorithm (constraint propagation + backtracking)
- Produces identical results to the C++ version
- Maintains the same command-line interface and input/output format
- Replaces manual memory management with Rust's safe ownership system

## License

GPL-3.0-or-later (same as original)