#define NUMBER_PLACER \
  "Al Klimov's Number Placer"
#define NUMBER_PLACER_VERSION \
  "1.0.27"
#define NUMBER_PLACER_COPYRIGHT \
  "Copyright (C) 2013-2014  Alexander A. Klimov"

/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Written in C++11

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <stdexcept>
/* invalid_argument
   out_of_range
   length_error */
#include <new>
// bad_alloc
#include <cstdlib>
/* EXIT_SUCCESS
   EXIT_FAILURE */
#include <cstddef>
// size_t

using namespace std;
using uint_sudoku_t = unsigned long long;

bool
    sudokuX = false,
    sudokuFail;
uint_sudoku_t
    sudokuSize[4],
    sudokuStrSize[2];
vector<uint_sudoku_t>
    sudokuContent,
    sudokuXPosition[2];
vector<vector<bool>>
    sudokuPossibilities;
vector<vector<uint_sudoku_t>>
    sudokuPosition[3];
vector<array<uint_sudoku_t,2>>
    sudokuAddress[3];
vector<string>
    argv;
map<uint_sudoku_t,uint_sudoku_t>
    sudokuXAddressMap[2];
vector<bool>
    sudokuXAddressValid[2];

inline void invalid_cmd_arg(int, string);
inline bool getNumberPossibility(uint_sudoku_t, uint_sudoku_t);
inline bool sudokuDone();
void setNumber(uint_sudoku_t, uint_sudoku_t);
void sudokuPrint(bool);
void sudokuTest();
bool sudokuTest(uint_sudoku_t);
bool sudokuCheck();
bool modNumber(uint_sudoku_t, uint_sudoku_t);
bool sudokuXAddress(uint_sudoku_t, unsigned char, uint_sudoku_t&);
uint_sudoku_t sudokuCount();
uint_sudoku_t sudokuCount(uint_sudoku_t);
uint_sudoku_t uint_digits(uint_sudoku_t);

int main(int argc, char** _argv) {
    cerr << NUMBER_PLACER "  "
            NUMBER_PLACER_VERSION "\n"
            NUMBER_PLACER_COPYRIGHT "\n" << endl;
    argv.resize(argc);
    for (decltype(argc) i = 0; i < argc; ++i)
        argv[i] = _argv[i];
    if (argc > 4) {
        cerr << "Error: '" << argv[0] << "' takes at most 3 command-line arguments (" << (argc - 1) << " given)" << endl;
        return EXIT_FAILURE;
    }
    if (argc > 1) {
        if (argv[argc-1] == "X" || argv[argc-1] == "x")
            sudokuX = true;
        else if (argc == 4) {
            invalid_cmd_arg(3, "Must be 'X' or 'x'!");
            return EXIT_FAILURE;
        }
    }
    for (signed char i = 0; i < 2; ++i) {
        if (argc - (sudokuX ? 2 : 1) < i + 1)
            sudokuSize[i] = i ? sudokuSize[0] : 3u;
        else {
            try {
                if ((sudokuSize[i] = stoull(argv[i+1])) < 2u)
                    throw invalid_argument("");
            } catch (const invalid_argument& e) {
                (void)e;
                invalid_cmd_arg(i + 1,
                    (!sudokuX && argc - 1 == i + 1)
                    ? "Must be an integer >= 2 or 'X' or 'x'!"
                    : "Must be an integer >= 2!");
                return EXIT_FAILURE;
            } catch (const out_of_range& e) {
                (void)e;
                invalid_cmd_arg(i + 1, "Not an integer or out of range!");
                return EXIT_FAILURE;
            }
        }
    }
    sudokuSize[2] = sudokuSize[0] * sudokuSize[1];
    sudokuStrSize[1] = (sudokuStrSize[0] = uint_digits(sudokuSize[2])) * (sudokuSize[3] = sudokuSize[2] * sudokuSize[2]);
    try {
        if (!(sudokuSize[0]    &&
              sudokuSize[1]    &&
              sudokuSize[2]    &&
              sudokuSize[3]    &&
              sudokuStrSize[0] &&
              sudokuStrSize[1]))
            throw length_error("");
        if (!(sudokuSize[3] / sudokuSize[2] == sudokuSize[2] &&
              sudokuSize[2] / sudokuSize[1] == sudokuSize[0] &&
           sudokuStrSize[1] / sudokuSize[3] == sudokuStrSize[0]))
            throw length_error("");
        cerr << "Sudoku size: " << sudokuSize[0] << "x" << sudokuSize[1] << " (" << sudokuSize[2] << "x" << sudokuSize[2] << ")\n"
                "X-Sudoku: " << (sudokuX ? "yes" : "no") << "\n"
                "\n"
                "Preparing... ";
        sudokuPossibilities.resize(sudokuSize[3]);
        sudokuContent      .resize(sudokuSize[3]);
        for (uint_sudoku_t i = 0u; i < sudokuSize[3]; ++i)
            sudokuPossibilities[i].resize(sudokuSize[2]);
        for (unsigned char i = 0u; i < 3u; ++i) {
            sudokuAddress[i].resize(sudokuSize[3]);
            sudokuPosition[i].resize(sudokuSize[2]);
            for (uint_sudoku_t j = 0u, k; j < sudokuSize[2]; ++j) {
                sudokuPosition[i][j].resize(sudokuSize[2]);
                for (k = 0u; k < sudokuSize[2]; ++k) {
                    if (i == 2u)
                        sudokuPosition[2][j][k] =
                            j / sudokuSize[1] * sudokuSize[2] * sudokuSize[1] +
                            k / sudokuSize[0] * sudokuSize[2] +
                            j % sudokuSize[1] * sudokuSize[0] +
                            k % sudokuSize[0];
                    else sudokuPosition[i][j][k] = i ? (k * sudokuSize[2] + j)
                                                     : (j * sudokuSize[2] + k);
                    sudokuAddress[i][ sudokuPosition[i][j][k] ][0] = j;
                    sudokuAddress[i][ sudokuPosition[i][j][k] ][1] = k;
                }
            }
        }
        if (sudokuX)
            for (unsigned char i = 0u; i < 2u; ++i) {
                sudokuXPosition    [i].resize(sudokuSize[2]);
                sudokuXAddressValid[i].resize(sudokuSize[3]);
                for (uint_sudoku_t j = 0u; j < sudokuSize[3]; ++j)
                    sudokuXAddressValid[i][j] = false;
                for (uint_sudoku_t j = 0u; j < sudokuSize[2]; ++j) {
                    sudokuXAddressValid[i][
                        sudokuXPosition[i][j] = j * sudokuSize[2] + (i ? (sudokuSize[2] - 1u - j) : j)
                    ] = true;
                    sudokuXAddressMap[i][ sudokuXPosition[i][j] ] = j;
                }
            }
    } catch (const bad_alloc& e) {
        (void)e;
        cerr << "Error: Out of memory!" << endl;
        return EXIT_FAILURE;
    } catch (const length_error& e) {
        (void)e;
        cerr << "Error: Too large Sudoku!" << endl;
        return EXIT_FAILURE;
    }
    cerr << "done." << endl;
    bool firstLine = true,
         sudokuSuccess;
    string sudokuInStr;
    size_t sudokuInStrLen;
    for (;;) {
        getline(cin, sudokuInStr);
        sudokuInStrLen = sudokuInStr.length();
        if (sudokuInStrLen) {
            if (sudokuInStrLen == sudokuStrSize[1]) {
                for (uint_sudoku_t i = 0u; i < sudokuStrSize[1]; ++i)
                    if (!('0' <= sudokuInStr[i] &&
                                 sudokuInStr[i] <= '9')) {
                        cerr << "Error: Invalid input!\n"
                                "Each character must be a decimal number!" << endl;
                        return EXIT_FAILURE;
                    }
                {
                    string s;
                    for (uint_sudoku_t i = 0u, j; i < sudokuSize[3]; ++i) {
                        s = sudokuInStr.substr(i * sudokuStrSize[0], sudokuStrSize[0]);
                        try {
                            if ((j = stoull(s)) > sudokuSize[2]) {
                                cerr << "Error: Invalid input!\n"
                                        "Number " << j << " out of range (0-" << sudokuSize[2] << ")!" << endl;
                                return EXIT_FAILURE;
                            } else setNumber(i, j);
                        } catch (const out_of_range& e) {
                            (void)e;
                            cerr << "Error: Invalid input: '" << s << "'\n"
                                    "Integer out of range!" << endl;
                            return EXIT_FAILURE;
                        }
                    }
                }
                auto i = sudokuCount();
                if (i) {
                    if (sudokuCheck()) {
                        if (i == sudokuSize[3])
                            sudokuPrint(true);
                        else {
                            sudokuFail = false;
                            unsigned char a;
                            uint_sudoku_t b, c, d, e, f;
                            do {
                                sudokuSuccess = false;
                                for (a = 0u; a <            3u; ++a)
                                for (b = 0u; b < sudokuSize[2]; ++b)
                                for (c = 0u; c < sudokuSize[2]; ++c) {
                                    if (!sudokuContent[ d = sudokuPosition[a][b][c] ]) {
                                        for (e = 0u; e < sudokuSize[2]; ++e)
                                            if (e != c)
                                            if (sudokuContent[ f = sudokuPosition[a][b][e] ])
                                            if (modNumber(d, sudokuContent[f])) {
                                                if (sudokuDone())
                                                    goto SudokuInterrupt;
                                                sudokuSuccess = true;
                                            }
                                        for (e = 1u; e <= sudokuSize[2]; ++e)
                                            if (getNumberPossibility(d, e)) {
                                                for (f = 0u; f < sudokuSize[2]; ++f)
                                                    if (f != c && getNumberPossibility(sudokuPosition[a][b][f], e))
                                                        goto ContinueParentLoop1;
                                                setNumber(d, e);
                                                if (sudokuDone())
                                                    goto SudokuInterrupt;
                                                sudokuSuccess = true;
                                                break;
                                                ContinueParentLoop1:;
                                            }
                                    }
                                }
                                if (sudokuX)
                                    for (a = 0u; a <            2u; ++a)
                                    for (c = 0u; c < sudokuSize[2]; ++c) {
                                        if (!sudokuContent[ d = sudokuXPosition[a][c] ]) {
                                            for (e = 0u; e < sudokuSize[2]; ++e)
                                                if (e != c)
                                                if (sudokuContent[ f = sudokuXPosition[a][e] ])
                                                if (modNumber(d, sudokuContent[f])) {
                                                    if (sudokuDone())
                                                        goto SudokuInterrupt;
                                                    sudokuSuccess = true;
                                                }
                                            for (e = 1u; e <= sudokuSize[2]; ++e)
                                                if (getNumberPossibility(d, e)) {
                                                    for (f = 0u; f < sudokuSize[2]; ++f)
                                                        if (f != c && getNumberPossibility(sudokuXPosition[a][f], e))
                                                            goto ContinueParentLoop2;
                                                    setNumber(d, e);
                                                    if (sudokuDone())
                                                        goto SudokuInterrupt;
                                                    sudokuSuccess = true;
                                                    break;
                                                    ContinueParentLoop2:;
                                                }
                                        }
                                    }
                            } while (sudokuSuccess);
                            sudokuTest();
                            SudokuInterrupt:
                            if (!sudokuFail)
                                sudokuFail = !sudokuCheck();
                            sudokuPrint(!sudokuFail);
                        }
                    } else sudokuPrint(false);
                } else sudokuPrint(false);
                if (cin.eof()) {
                    cerr << "End of file.\n"
                            "Warning: No EOL @ EOF" << endl;
                    return EXIT_SUCCESS;
                }
                if (firstLine)
                    firstLine = false;
            } else {
                cerr << "Error: Invalid input!\n"
                        "Each line must be " << sudokuStrSize[1] << " characters long!" << endl;
                return EXIT_FAILURE;
            }
        } else {
            if (cin.eof()) {
                cerr << (firstLine ? "EOF @ first line -- nothing to do." : "End of file.") << endl;
                return EXIT_SUCCESS;
            } else {
                cerr << "Warning: Ignoring blank line" << endl;
                if (firstLine)
                    firstLine = false;
            }
        }
    }
}

void invalid_cmd_arg(int i, string s) {
    cerr << "Error: Invalid command-line argument (" << i << "): '" << argv[i] << "'\n"
         << s << endl;
}

void setNumber(uint_sudoku_t n, uint_sudoku_t x) {
    sudokuContent[n] = x;
    for (uint_sudoku_t i = 0u; i < sudokuSize[2]; ++i)
        sudokuPossibilities[n][i] = x ? (i == x - 1u) : true;
}

uint_sudoku_t sudokuCount(uint_sudoku_t n) {
    uint_sudoku_t x = 0u;
    for (uint_sudoku_t i = 0u; i < sudokuSize[2]; ++i)
        if (sudokuPossibilities[n][i])
        if (++x > 1u)
            break;
    return x;
}

uint_sudoku_t sudokuCount() {
    uint_sudoku_t x = 0u;
    for (uint_sudoku_t i = 0u; i < sudokuSize[3]; ++i)
        if (sudokuContent[i])
            ++x;
    return x;
}

bool getNumberPossibility(uint_sudoku_t n, uint_sudoku_t x) {
    return sudokuPossibilities[n][x-1u];
}

uint_sudoku_t uint_digits(uint_sudoku_t n) {
    uint_sudoku_t x = 0u;
    do ++x;
    while (n /= 10u);
    return x;
}

void sudokuPrint(bool b) {
    if (b)
        for (uint_sudoku_t i = 0u, k; i < sudokuSize[3]; ++i) {
            for (k = uint_digits(sudokuContent[i]); k < sudokuStrSize[0]; ++k)
                cout << '0';
            cout << sudokuContent[i];
        }
    else
        for (uint_sudoku_t i = 0u; i < sudokuStrSize[1]; ++i)
            cout << '0';
    cout << endl;
}

bool sudokuCheck() {
    unsigned char i;
    uint_sudoku_t j, k, l, m;
    for (i = 0u; i <                 3u; ++i)
    for (j = 0u; j < sudokuSize[2]     ; ++j)
    for (k = 0u; k < sudokuSize[2] - 1u; ++k) {
        if (sudokuContent[ l = sudokuPosition[i][j][k] ])
            for (m = k + 1u; m < sudokuSize[2]; ++m)
                if (sudokuContent[l] == sudokuContent[ sudokuPosition[i][j][m] ])
                    return false;
    }
    if (sudokuX)
        for (i = 0u; i <                 2u; ++i)
        for (k = 0u; k < sudokuSize[2] - 1u; ++k) {
            if (sudokuContent[ l = sudokuXPosition[i][k] ])
                for (m = k + 1u; m < sudokuSize[2]; ++m)
                    if (sudokuContent[l] == sudokuContent[ sudokuXPosition[i][m] ])
                        return false;
        }
    return true;
}

bool sudokuDone() {
    return sudokuFail || sudokuCount() == sudokuSize[3];
}

bool modNumber(uint_sudoku_t n, uint_sudoku_t x) {
    bool b = sudokuPossibilities[n][--x];
    if (b) {
        sudokuPossibilities[n][x] = false;
        auto i = sudokuCount(n);
        if (i) {
            if (i == 1u) {
                uint_sudoku_t j = 1u;
                while (!getNumberPossibility(n, j))
                    ++j;
                sudokuContent[n] = j;
            }
        } else sudokuFail = true;
    }
    return b;
}

bool sudokuXAddress(uint_sudoku_t x, unsigned char a, uint_sudoku_t& b) {
    if (!sudokuXAddressValid[a][x])
        return false;
    b = sudokuXAddressMap[a].at(x);
    return true;
}

void sudokuTest() {
    for (uint_sudoku_t i = 0u; i < sudokuSize[3]; ++i)
        sudokuContent[i] = 0u;
    for (uint_sudoku_t n = 0u;;) {
        while (++sudokuContent[n] <= sudokuSize[2])
            if (getNumberPossibility(n, sudokuContent[n]))
            if (sudokuTest(n)) {
                if (n < sudokuSize[3] - 1u)
                    ++n;
                else return;
            }
        if (n)
            sudokuContent[n--] = 0u;
        else {
            sudokuFail = true;
            return;
        }
    }
}

bool sudokuTest(uint_sudoku_t n) {
    unsigned char a;
    uint_sudoku_t b, c, d;
    for (a = 0u; a <            3u; ++a)
    for (b = 0u; b < sudokuSize[2]; ++b)
        if (b != sudokuAddress[a][n][1]) {
            c = sudokuPosition[a][ sudokuAddress[a][n][0] ][b];
            if (sudokuContent[c] &&
                sudokuContent[c] == sudokuContent[n])
                return false;
        }
    if (sudokuX)
        for (a = 0u; a < 2u; ++a)
            if (sudokuXAddress(n, a, b))
                for (c = 0u; c < sudokuSize[2]; ++c)
                    if (c != b) {
                        d = sudokuXPosition[a][c];
                        if (sudokuContent[d] &&
                            sudokuContent[d] == sudokuContent[n])
                            return false;
                    }
    return true;
}

#undef NUMBER_PLACER
#undef NUMBER_PLACER_VERSION
#undef NUMBER_PLACER_COPYRIGHT
