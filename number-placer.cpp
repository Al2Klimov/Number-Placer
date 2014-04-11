/* Al Klimov's Number Placer  1.0.20 (2014-04-11)
 * Copyright (C) 2013-2014  Alexander A. Klimov
 * Written in C++11
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <exception>
#include <stdexcept>
#include <new>
#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
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
    sudokuTestContent,
    sudokuXPosition[2];
vector<vector<bool>>
    sudokuPossibilities;
vector<vector<uint_sudoku_t>>
    sudokuPosition[3];
vector<array<uint_sudoku_t,2>>
    sudokuAddress[3];
vector<string>
    argv;

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

class sudoku_interrupt: public exception
{} sudokuInterrupt;

int main(int argc, char** _argv) {
    cerr << "Al Klimov's Number Placer  1.0.20\n"
            "Copyright (C) 2013-2014  Alexander A. Klimov\n" << endl;
    argv.resize(argc);
    for (decltype(argc) i = 0; i < argc; i++)
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
    for (signed char i = 0; i < 2; i++) {
        if (argc - (sudokuX ? 2 : 1) < i + 1)
            sudokuSize[i] = i ? sudokuSize[0] : 3;
        else {
            try {
                if ((sudokuSize[i] = stoull(argv[i+1])) < 2)
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
        sudokuTestContent  .resize(sudokuSize[3]);
        for (uint_sudoku_t i = 0; i < sudokuSize[3]; i++)
            sudokuPossibilities[i].resize(sudokuSize[2]);
        for (unsigned char i = 0; i < 3; i++) {
            sudokuAddress[i].resize(sudokuSize[3]);
            sudokuPosition[i].resize(sudokuSize[2]);
            for (uint_sudoku_t j = 0, k; j < sudokuSize[2]; j++) {
                sudokuPosition[i][j].resize(sudokuSize[2]);
                for (k = 0; k < sudokuSize[2]; k++) {
                    if (i == 2)
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
            for (unsigned char i = 0; i < 2; i++) {
                sudokuXPosition[i].resize(sudokuSize[2]);
                for (uint_sudoku_t j = 0; j < sudokuSize[2]; j++)
                    sudokuXPosition[i][j] = j * sudokuSize[2] + (i ? (sudokuSize[2] - 1 - j) : j);
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
    for (;;) {
        getline(cin, sudokuInStr);
        auto sudokuInStrLen = sudokuInStr.length();
        if (sudokuInStrLen) {
            if (sudokuInStrLen == sudokuStrSize[1]) {
                for (uint_sudoku_t i = 0; i < sudokuStrSize[1]; i++)
                    if (!(48 <= sudokuInStr[i] && sudokuInStr[i] <= 57)) {
                        cerr << "Error: Invalid input!\n"
                                "Each character must be a decimal number!" << endl;
                        return EXIT_FAILURE;
                    }
                for (uint_sudoku_t i = 0, j; i < sudokuSize[3]; i++) {
                    auto s = sudokuInStr.substr(i * sudokuStrSize[0], sudokuStrSize[0]);
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
                auto i = sudokuCount();
                if (i) {
                    if (sudokuCheck()) {
                        if (i == sudokuSize[3])
                            sudokuPrint(true);
                        else {
                            sudokuFail = false;
                            unsigned char a;
                            uint_sudoku_t b, c, d, e, f;
                            try {
                                do {
                                    sudokuSuccess = false;
                                    for (a = 0; a <             3; a++)
                                    for (b = 0; b < sudokuSize[2]; b++)
                                    for (c = 0; c < sudokuSize[2]; c++) {
                                        if (sudokuCount(d = sudokuPosition[a][b][c]) > 1) {
                                            for (e = 0; e < sudokuSize[2]; e++)
                                                if (e != c)
                                                if (sudokuCount(f = sudokuPosition[a][b][e]) == 1)
                                                if (modNumber(d, sudokuContent[f]))
                                                    sudokuSuccess = true;
                                            for (e = 1; e <= sudokuSize[2]; e++)
                                                if (getNumberPossibility(d, e)) {
                                                    for (f = 0; f < sudokuSize[2]; f++)
                                                        if (f != c && getNumberPossibility(sudokuPosition[a][b][f], e))
                                                            goto ContinueParentLoop1;
                                                    setNumber(d, e);
                                                    if (sudokuDone())
                                                        throw sudokuInterrupt;
                                                    sudokuSuccess = true;
                                                    break;
                                                    ContinueParentLoop1:;
                                                }
                                        }
                                    }
                                    if (sudokuX)
                                        for (a = 0; a <             2; a++)
                                        for (c = 0; c < sudokuSize[2]; c++) {
                                            if (sudokuCount(d = sudokuXPosition[a][c]) > 1) {
                                                for (e = 0; e < sudokuSize[2]; e++)
                                                    if (e != c)
                                                    if (sudokuCount(f = sudokuXPosition[a][e]) == 1)
                                                    if (modNumber(d, sudokuContent[f]))
                                                        sudokuSuccess = true;
                                                for (e = 1; e <= sudokuSize[2]; e++)
                                                    if (getNumberPossibility(d, e)) {
                                                        for (f = 0; f < sudokuSize[2]; f++)
                                                            if (f != c && getNumberPossibility(sudokuXPosition[a][f], e))
                                                                goto ContinueParentLoop2;
                                                        setNumber(d, e);
                                                        if (sudokuDone())
                                                            throw sudokuInterrupt;
                                                        sudokuSuccess = true;
                                                        break;
                                                        ContinueParentLoop2:;
                                                    }
                                            }
                                        }
                                } while (sudokuSuccess);
                                sudokuTest();
                            } catch (const sudoku_interrupt& e) {
                                (void)e;
                            }
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
    for (uint_sudoku_t i = 0; i < sudokuSize[2]; i++)
        sudokuPossibilities[n][i] = x ? (i == x - 1) : true;
}

uint_sudoku_t sudokuCount(uint_sudoku_t n) {
    uint_sudoku_t x = 0;
    for (uint_sudoku_t i = 0; i < sudokuSize[2]; i++)
        if (sudokuPossibilities[n][i])
            x++;
    return x;
}

uint_sudoku_t sudokuCount() {
    uint_sudoku_t x = 0;
    for (uint_sudoku_t i = 0; i < sudokuSize[3]; i++)
        if (sudokuCount(i) == 1)
            x++;
    return x;
}

bool getNumberPossibility(uint_sudoku_t n, uint_sudoku_t x) {
    return sudokuPossibilities[n][x-1];
}

uint_sudoku_t uint_digits(uint_sudoku_t n) {
    uint_sudoku_t x = 0;
    do x++;
    while (n /= 10);
    return x;
}

void sudokuPrint(bool b) {
    if (b)
        for (uint_sudoku_t i = 0; i < sudokuSize[3]; i++) {
            for (auto k = uint_digits(sudokuContent[i]); k < sudokuStrSize[0]; k++)
                cout << 0;
            cout << sudokuContent[i];
        }
    else for (uint_sudoku_t i = 0; i < sudokuStrSize[1]; i++)
             cout << 0;
    cout << endl;
}

bool sudokuCheck() {
    unsigned char i;
    uint_sudoku_t j, k, l, m;
    for (i = 0; i <                 3; i++)
    for (j = 0; j < sudokuSize[2]    ; j++)
    for (k = 0; k < sudokuSize[2] - 1; k++) {
        if (sudokuContent[l = sudokuPosition[i][j][k]])
            for (m = k + 1; m < sudokuSize[2]; m++)
                if (sudokuContent[l] == sudokuContent[sudokuPosition[i][j][m]])
                    return false;
    }
    if (sudokuX)
        for (i = 0; i <                 2; i++)
        for (k = 0; k < sudokuSize[2] - 1; k++) {
            if (sudokuContent[l = sudokuXPosition[i][k]])
                for (m = k + 1; m < sudokuSize[2]; m++)
                    if (sudokuContent[l] == sudokuContent[sudokuXPosition[i][m]])
                        return false;
        }
    return true;
}

bool sudokuDone() {
    return sudokuFail || sudokuCount() == sudokuSize[3];
}

bool modNumber(uint_sudoku_t n, uint_sudoku_t x) {
    bool b = sudokuPossibilities[n][--x];
    if (b)
        sudokuPossibilities[n][x] = false;
    auto i = sudokuCount(n);
    if (i) {
        if (i == 1) {
            uint_sudoku_t j = 1;
            while (!getNumberPossibility(n, j))
                j++;
            sudokuContent[n] = j;
        }
    } else sudokuFail = true;
    if (sudokuDone())
        throw sudokuInterrupt;
    return b;
}

bool sudokuXAddress(uint_sudoku_t x, unsigned char a, uint_sudoku_t& b) {
    for (uint_sudoku_t i = 0; i < sudokuSize[2]; i++)
        if (sudokuXPosition[a][i] == x) {
            b = i;
            return true;
        }
    return false;
}

void sudokuTest() {
    for (uint_sudoku_t i = 0; i < sudokuSize[3]; i++)
        sudokuTestContent[i] = 0;
    for (uint_sudoku_t n = 0;;) {
        while (++sudokuTestContent[n] <= sudokuSize[2])
            if (getNumberPossibility(n, sudokuTestContent[n]))
            if (sudokuTest(n)) {
                if (n < sudokuSize[3] - 1)
                    n++;
                else {
                    for (uint_sudoku_t i = 0; i < sudokuSize[3]; i++)
                        setNumber(i, sudokuTestContent[i]);
                    return;
                }
            }
        if (n)
            sudokuTestContent[n--] = 0;
        else {
            sudokuFail = true;
            return;
        }
    }
}

bool sudokuTest(uint_sudoku_t n) {
    unsigned char a;
    uint_sudoku_t b, c, d;
    for (a = 0; a <             3; a++)
    for (b = 0; b < sudokuSize[2]; b++)
        if (b != sudokuAddress[a][n][1]) {
            c = sudokuPosition[a][sudokuAddress[a][n][0]][b];
            if (sudokuTestContent[c] &&
                sudokuTestContent[c] == sudokuTestContent[n])
                return false;
        }
    if (sudokuX)
        for (a = 0; a < 2; a++)
            if (sudokuXAddress(n, a, b))
                for (c = 0; c < sudokuSize[2]; c++)
                    if (c != b) {
                        d = sudokuXPosition[a][c];
                        if (sudokuTestContent[d] &&
                            sudokuTestContent[d] == sudokuTestContent[n])
                            return false;
                    }
    return true;
}
