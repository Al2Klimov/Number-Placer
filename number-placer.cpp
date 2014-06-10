#define NUMBER_PLACER \
  "Al Klimov's Number Placer  1.0.32" "\n" \
  "Copyright (C) 2013-2014  Alexander A. Klimov"
/*
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

// Written in C++11

#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;
using std::to_string;

#include <sstream>
using std::istringstream;

#include <new>
using std::bad_alloc;

#include <cstdio>
// sprintf

#include <climits>
// CHAR_MIN

#include <cstdlib>
/* EXIT_SUCCESS
   EXIT_FAILURE */

#include <cstddef>
// size_t

bool
    sudokuX = false,
    sudokuFail,
  **sudokuPossibilities,
   *sudokuXAddressValid[2];
size_t
    sudokuSize[4],
    sudokuStrSize[2],
   *sudokuContent,
   *sudokuAddress[3][2],
  **sudokuPosition[3],
   *sudokuXPosition[2];

#define SudokuAddress(x, y, z) \
        sudokuAddress[x][z][y]

void setNumber(size_t, size_t);
void sudokuPrint(bool);
void sudokuTest();
bool sudokuTest(size_t);
bool sudokuCheck();
bool modNumber(size_t, size_t);
bool sudokuXAddress(size_t, unsigned char, size_t&);
unsigned char sudokuCount(size_t);
size_t sudokuCount();
size_t uIntDigits(size_t);
size_t sToSize_t(const string&);
string repr(const string&);

#define InvalidCmdArg(i, s) \
        cerr << "Error: Invalid command-line argument " \
                "(" << (i) << "): " << repr(args[i]) << "\n" \
             << (s) << endl; \
        return EXIT_FAILURE

#define SudokuPossibility(x, y) \
        sudokuPossibilities[x][ (y) - 1u ]

#define SudokuDone() \
        (sudokuFail || sudokuCount() == sudokuSize[3])

int main(int argc, char** argv) {
    cerr << NUMBER_PLACER "\n" << endl;
    if (argc > 4) {
        cerr << "Error: '" << argv[0] << "' takes at most 3 command-line arguments "
                "(" << (argc - 1) << " given)" << endl;
        return EXIT_FAILURE;
    }
    try {
        string *args = new string[argc];
        for (decltype(argc) i = 0; i < argc; ++i)
            args[i] = argv[i];
        if (argc > 1) {
            if (args[argc-1] == "X" || args[argc-1] == "x")
                sudokuX = true;
            else if (argc == 4) {
                InvalidCmdArg(3, "Must be \"X\" or \"x\"!");
            }
        }
        for (signed char i = 0; i < 2; ++i) {
            if (argc - (sudokuX ? 2 : 1) < i + 1)
                sudokuSize[i] = i ? sudokuSize[0] : 3u;
            else {
                try {
                    if ((sudokuSize[i] = sToSize_t(args[i+1])) < 2u)
                        throw args[i+1];
                } catch (const string& s) {
                    (void)s;
                    InvalidCmdArg(
                        i + 1,
                        "Must be an integer (2 <= n <= " + to_string(size_t(-1)) + (
                            (!sudokuX && argc - 1 == i + 1)
                            ? "), \"X\" or \"x\"!"
                            : ")!"
                        )
                    );
                }
            }
        }
        delete[] args;
        sudokuSize[2] = sudokuSize[0] * sudokuSize[1];
        sudokuSize[3] = sudokuSize[2] * sudokuSize[2];
        sudokuStrSize[0] = uIntDigits(sudokuSize[2]);
        sudokuStrSize[1] = sudokuStrSize[0] * sudokuSize[3];
        if (!(
            sudokuSize[0] &&
            sudokuSize[1] &&
            sudokuSize[2] &&
            sudokuSize[3] &&
            sudokuStrSize[0] &&
            sudokuStrSize[1] &&
            sudokuSize[3] / sudokuSize[2] == sudokuSize[2] &&
            sudokuSize[2] / sudokuSize[1] == sudokuSize[0] &&
         sudokuStrSize[1] / sudokuSize[3] == sudokuStrSize[0]
        )) {
            cerr << "Error: Too large Sudoku!" << endl;
            return EXIT_FAILURE;
        }
        cerr << "Sudoku size: " << sudokuSize[0] << "x" << sudokuSize[1] << " "
                            "(" << sudokuSize[2] << "x" << sudokuSize[2] << ")\n"
                "X-Sudoku: " << (sudokuX ? "yes" : "no") << "\n\n"
                "Preparing... ";
        sudokuContent = new size_t[sudokuSize[3]];
        sudokuPossibilities = new bool*[sudokuSize[3]];
        for (size_t i = 0u; i < sudokuSize[3]; ++i)
            sudokuPossibilities[i] = new bool[sudokuSize[2]];
        for (unsigned char i = 0u; i < 3u; ++i) {
            for (unsigned char j = 0u; j < 2u; ++j)
                sudokuAddress[i][j] = new size_t[sudokuSize[3]];
            sudokuPosition[i] = new size_t*[sudokuSize[2]];
            for (size_t j = 0u, k; j < sudokuSize[2]; ++j) {
                sudokuPosition[i][j] = new size_t[sudokuSize[2]];
                for (k = 0u; k < sudokuSize[2]; ++k) {
                    if (i == 2u)
                        sudokuPosition[2][j][k] =
                            j / sudokuSize[1] * sudokuSize[2] * sudokuSize[1] +
                            k / sudokuSize[0] * sudokuSize[2] +
                            j % sudokuSize[1] * sudokuSize[0] +
                            k % sudokuSize[0];
                    else sudokuPosition[i][j][k] = i ? (k * sudokuSize[2] + j)
                                                     : (j * sudokuSize[2] + k);
                    SudokuAddress(i, sudokuPosition[i][j][k], 0) = j;
                    SudokuAddress(i, sudokuPosition[i][j][k], 1) = k;
                }
            }
        }
        if (sudokuX)
            for (unsigned char i = 0u; i < 2u; ++i) {
                sudokuXPosition[i] = new size_t[sudokuSize[2]];
                sudokuXAddressValid[i] = new bool[sudokuSize[3]];
                for (size_t j = 0u; j < sudokuSize[3]; ++j)
                    sudokuXAddressValid[i][j] = false;
                for (size_t j = 0u; j < sudokuSize[2]; ++j)
                    sudokuXAddressValid[i][
                        sudokuXPosition[i][j] = j * sudokuSize[2] + (i ? (sudokuSize[2] - 1u - j) : j)
                    ] = true;
            }
    } catch (const bad_alloc& e) {
        (void)e;
        cerr << "Error: Out of memory!" << endl;
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
                {
                    string s;
                    for (size_t i = 0u, j; i < sudokuSize[3]; ++i) {
                        s = sudokuInStr.substr(i * sudokuStrSize[0], sudokuStrSize[0]);
                        try {
                            if ((j = sToSize_t(s)) > sudokuSize[2])
                                throw s;
                            setNumber(i, j);
                        } catch (const string& S) {
                            cerr << "Error: Invalid input: " << repr(S) << "\n"
                                    "Must be an integer (0 <= n <= " << sudokuSize[2] << ")!" << endl;
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
                            size_t b, c, d, e, f;
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
                                                if (SudokuDone())
                                                    goto SudokuInterrupt;
                                                sudokuSuccess = true;
                                            }
                                        for (e = 1u; e <= sudokuSize[2]; ++e)
                                            if (SudokuPossibility(d, e)) {
                                                for (f = 0u; f < sudokuSize[2]; ++f)
                                                    if (f != c && SudokuPossibility(sudokuPosition[a][b][f], e))
                                                        goto ContinueParentLoop1;
                                                setNumber(d, e);
                                                if (SudokuDone())
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
                                                    if (SudokuDone())
                                                        goto SudokuInterrupt;
                                                    sudokuSuccess = true;
                                                }
                                            for (e = 1u; e <= sudokuSize[2]; ++e)
                                                if (SudokuPossibility(d, e)) {
                                                    for (f = 0u; f < sudokuSize[2]; ++f)
                                                        if (f != c && SudokuPossibility(sudokuXPosition[a][f], e))
                                                            goto ContinueParentLoop2;
                                                    setNumber(d, e);
                                                    if (SudokuDone())
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

#undef NUMBER_PLACER
#undef InvalidCmdArg
#undef SudokuDone

size_t sToSize_t(const string& s) {
    static istringstream iss;
    if (s.empty())
        throw s;
    for (auto i = s.begin(); i != s.end(); ++i)
        if (!('0' <= *i &&
                     *i <= '9'))
            throw s;
    size_t i;
    iss.clear();
    iss.str(s);
    if ((iss >> i).fail())
        throw s;
    return i;
}

string repr(const string& s) {
    string r = "\"";
    char c[3];
    for (auto i = s.begin(); i != s.end(); ++i)
        switch (*i) {
            case '\a': r += "\\a";  break;
            case '\b': r += "\\b";  break;
            case '\f': r += "\\f";  break;
            case '\n': r += "\\n";  break;
            case '\r': r += "\\r";  break;
            case '\t': r += "\\t";  break;
            case '\v': r += "\\v";  break;
            case '\\': r += "\\\\"; break;
            case '"':  r += "\\\""; break;
            default:
                if (' ' <= *i &&
                           *i <= '~')
                    r += *i;
                else {
                    sprintf(
                        c, "%02X",
#if CHAR_MIN == 0
                        int(*i)
#else
                        (int(*i) + 256) % 256
#endif
                    );
                    (r += "\\x") += c;
                }
        }
    return r + "\"";
}

void setNumber(size_t n, size_t x) {
    sudokuContent[n] = x;
    for (size_t i = 0u; i < sudokuSize[2]; ++i)
        sudokuPossibilities[n][i] = x ? (i == x - 1u) : true;
}

unsigned char sudokuCount(size_t n) {
    unsigned char x = 0u;
    for (size_t i = 0u; i < sudokuSize[2]; ++i)
        if (sudokuPossibilities[n][i])
        if (++x > 1u)
            break;
    return x;
}

size_t sudokuCount() {
    size_t x = 0u;
    for (size_t i = 0u; i < sudokuSize[3]; ++i)
        if (sudokuContent[i])
            ++x;
    return x;
}

size_t uIntDigits(size_t n) {
    size_t x = 0u;
    do ++x;
    while (n /= 10u);
    return x;
}

void sudokuPrint(bool b) {
    if (b)
        for (size_t i = 0u, k; i < sudokuSize[3]; ++i) {
            for (k = uIntDigits(sudokuContent[i]); k < sudokuStrSize[0]; ++k)
                cout << '0';
            cout << sudokuContent[i];
        }
    else
        for (size_t i = 0u; i < sudokuStrSize[1]; ++i)
            cout << '0';
    cout << endl;
}

bool sudokuCheck() {
    unsigned char i;
    size_t j, k, l, m;
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

bool modNumber(size_t n, size_t x) {
    bool b = sudokuPossibilities[n][--x];
    if (b) {
        sudokuPossibilities[n][x] = false;
        auto i = sudokuCount(n);
        if (i) {
            if (i == 1u) {
                size_t j = 1u;
                while (!SudokuPossibility(n, j))
                    ++j;
                sudokuContent[n] = j;
            }
        } else sudokuFail = true;
    }
    return b;
}

bool sudokuXAddress(size_t x, unsigned char a, size_t& b) {
    if (!sudokuXAddressValid[a][x])
        return false;
    b = a ? (x / (sudokuSize[2] - 1u) - 1u)
          : (x / (sudokuSize[2] + 1u));
    return true;
}

void sudokuTest() {
    for (size_t i = 0u; i < sudokuSize[3]; ++i)
        sudokuContent[i] = 0u;
    for (size_t n = 0u;;) {
        while (++sudokuContent[n] <= sudokuSize[2])
            if (SudokuPossibility(n, sudokuContent[n]))
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

bool sudokuTest(size_t n) {
    unsigned char a;
    size_t b, c, d;
    for (a = 0u; a <            3u; ++a)
    for (b = 0u; b < sudokuSize[2]; ++b)
        if (b != SudokuAddress(a, n, 1)) {
            c = sudokuPosition[a][ SudokuAddress(a, n, 0) ][b];
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

#undef SudokuAddress
#undef SudokuPossibility
