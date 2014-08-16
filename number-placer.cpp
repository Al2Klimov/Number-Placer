#define NUMBER_PLACER \
  "Al Klimov's Number Placer  1.0.36" "\n" \
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

#include <sstream>
using std::istringstream;
using std::ostringstream;

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

class SystemExit {
public:
    SystemExit();
    SystemExit(int);
    SystemExit(const string&);
    SystemExit(const SystemExit&);

    operator int (void) const;

    template<class T>
    SystemExit& operator << (T rhs);
private:
    int i;
    ostringstream oss;

    void init(int, const string&);
};

class NumberPlacer {
public:
    NumberPlacer(size_t, size_t, bool);
   ~NumberPlacer();

    NumberPlacer& operator << (size_t);
    bool          operator () (void);
    NumberPlacer& operator >> (size_t&);
private:
    bool
        X, fail,
      **possibilities,
       *XAddressValid[2];
    size_t
        A, B,
        input, output,
       *content,
       *address[3][2],
      **position[3],
       *XPosition[2];

    void number(size_t, size_t);
    size_t count(void);
    bool check();
    bool modNumber(size_t, size_t);
    unsigned char count(size_t);
    bool test(size_t);
    bool XAddress(size_t, unsigned char, size_t&);

    void dealloc(void);
    template<class T>
    void dealloc(T);
    template<class T>
    void dealloc(T, size_t);
};

size_t uIntDigits(size_t);
size_t sToSize_t(const string&);
string repr(const string&);
SystemExit invalidCmdArg(int, char**);

int main(int argc, char **argv) {
    cerr << NUMBER_PLACER "\n" << endl;
#undef NUMBER_PLACER
    try {
        if (argc > 4)
            throw SystemExit("Error: '")
                << argv[0] << "' takes at most 3 command-line arguments ("
                << (argc - 1) << " given)";
        try {
            bool sudokuX = false;
            string *args = new string[argc];
            for (decltype(argc) i = 0; i < argc; ++i)
                args[i] = argv[i];
            if (argc > 1) {
                if (args[argc-1] == "X" || args[argc-1] == "x")
                    sudokuX = true;
                else if (argc == 4)
                    throw invalidCmdArg(3, argv)
                        << "Must be \"X\" or \"x\"!";
            }
            size_t
                sudokuSize[4],
                sudokuStrSize[2];
            for (signed char i = 0; i < 2; ++i) {
                if (argc - (sudokuX ? 2 : 1) < i + 1)
                    sudokuSize[i] = i ? sudokuSize[0] : 3u;
                else {
                    try {
                        if ((sudokuSize[i] = sToSize_t(args[i+1])) < 2u)
                            throw args[i+1];
                    } catch (const string& s) {
                        (void)s;
                        throw invalidCmdArg(i + 1, argv)
                            << "Must be an integer (2 <= n <= "
                            << size_t(-1)
                            << (
                                (!sudokuX && argc - 1 == i + 1)
                                ? "), \"X\" or \"x\"!"
                                : ")!"
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
            )) throw SystemExit("Error: Too large Sudoku!");
            cerr << "Sudoku size: " << sudokuSize[0] << "x" << sudokuSize[1] << " "
                                "(" << sudokuSize[2] << "x" << sudokuSize[2] << ")\n"
                    "X-Sudoku: " << (sudokuX ? "yes" : "no") << "\n\n"
                    "Preparing... ";
            NumberPlacer sudoku (sudokuSize[0], sudokuSize[1], sudokuX);
            cerr << "done." << endl;
            bool firstLine = true;
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
                                    sudoku << j;
                                } catch (const string& S) {
                                    throw SystemExit("Error: Invalid input: ")
                                        << repr(S)
                                        << "\nMust be an integer (0 <= n <= "
                                        << sudokuSize[2] << ")!";
                                }
                            }
                        }
                        if (sudoku())
                            for (size_t i = 0u, j, k; i < sudokuSize[3]; ++i) {
                                sudoku >> j;
                                for (k = uIntDigits(j); k < sudokuStrSize[0]; ++k)
                                    cout << '0';
                                cout << j;
                            }
                        else
                            for (size_t i = 0u; i < sudokuStrSize[1]; ++i)
                                cout << '0';
                        cout << endl;
                        if (cin.eof()) {
                            cerr << "End of file.\n"
                                    "Warning: No EOL @ EOF" << endl;
                            return EXIT_SUCCESS;
                        }
                        if (firstLine)
                            firstLine = false;
                    } else throw SystemExit("Error: Invalid input!\nEach line must be ")
                        << sudokuStrSize[1]
                        << " characters long!";
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
        } catch (const bad_alloc& e) {
            (void)e;
            throw SystemExit("Error: Out of memory!");
        }
    } catch (const SystemExit& se) {
        return se;
    }
}

size_t sToSize_t(const string& s) {
    static istringstream iss;
    if (s.empty())
        throw s;
    for (const auto c : s)
        if (!('0' <= c &&
                     c <= '9'))
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
    char buf[3];
    for (const auto c : s)
        switch (c) {
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
                if (' ' <= c &&
                           c <= '~')
                    r += c;
                else {
                    sprintf(
                        buf, "%02X",
#if CHAR_MIN == 0
                        int(c)
#else
                        (int(c) + 256) % 256
#endif
                    );
                    (r += "\\x") += buf;
                }
        }
    return r + "\"";
}

size_t uIntDigits(size_t n) {
    size_t x = 0u;
    do ++x;
    while (n /= 10u);
    return x;
}

SystemExit invalidCmdArg(int argi, char **argv) {
    SystemExit se ("Error: Invalid command-line argument (");
    se << argi << "): " << repr(argv[argi]) << "\n";
    return se;
}

// SystemExit
SystemExit::SystemExit() {
    init(EXIT_SUCCESS, "");
}

SystemExit::SystemExit(int I) {
    init(I, "");
}

SystemExit::SystemExit(const string& S) {
    init(EXIT_FAILURE, S);
}

SystemExit::SystemExit(const SystemExit& se) {
    init(
        se.i,
        se.oss.str()
    );
}

SystemExit::operator int (void) const {
    cerr << oss.str() << endl;
    return i;
}

template<class T>
SystemExit& SystemExit::operator << (T rhs) {
    oss << rhs;
    return *this;
}

void SystemExit::init(int I, const string& s) {
    i = I;
    oss.str("");
    oss << s;
}
// ~SystemExit

// NumberPlacer
NumberPlacer::NumberPlacer(size_t a, size_t b, bool x) {
    A = a * b;
    input = 0u;
    output = B = A * A;
    X = x;

    possibilities = nullptr;
    content = nullptr;
    {
        unsigned char i, j;
        for (i = 0u; i < 3u; ++i) {
            for (j = 0u; j < 2u; ++j)
                address[i][j] = nullptr;
            position[i] = nullptr;
        }
        for (i = 0u; i < 2u; ++i) {
            XAddressValid[i] = nullptr;
            XPosition[i] = nullptr;
        }
    }

    try {
        content = new size_t[B];
        possibilities = new bool*[B];
        {
            size_t i;
            for (i = 0u; i < B; ++i)
                possibilities[i] = nullptr;
            for (i = 0u; i < B; ++i)
                possibilities[i] = new bool[A];
        }
        for (unsigned char i = 0u; i < 3u; ++i) {
            for (unsigned char j = 0u; j < 2u; ++j)
                address[i][j] = new size_t[B];
            position[i] = new size_t*[A];
            {
                size_t j, k;
                for (j = 0u; j < A; ++j)
                    position[i][j] = nullptr;
                for (j = 0u; j < A; ++j) {
                    position[i][j] = new size_t[A];
                    for (k = 0u; k < A; ++k) {
                        if (i == 2u)
                            position[2][j][k] = j / b * A * b
                                              + k / a * A
                                              + j % b * a
                                              + k % a;
                        else position[i][j][k] = i ? (k * A + j)
                                                   : (j * A + k);
                        address[i][0][ position[i][j][k] ] = j;
                        address[i][1][ position[i][j][k] ] = k;
                    }
                }
            }
        }
        if (x)
            for (unsigned char i = 0u; i < 2u; ++i) {
                XPosition[i] = new size_t[A];
                XAddressValid[i] = new bool[B];
                {
                    size_t j;
                    for (j = 0u; j < B; ++j)
                        XAddressValid[i][j] = false;
                    for (j = 0u; j < A; ++j)
                        XAddressValid[i][
                            XPosition[i][j] = j * A + (i ? (A - 1u - j) : j)
                        ] = true;
                }
            }
    } catch (...) {
        dealloc();
        throw;
    }
}

NumberPlacer::~NumberPlacer() {
    dealloc();
}

NumberPlacer& NumberPlacer::operator << (size_t x) {
    number(input++, x);
    return *this;
}

#define Possibility(x, y) \
        possibilities[x][ (y) - 1u ]

#define SudokuDone() \
        (fail || count() == B)

bool NumberPlacer::operator () (void) {
    input = output = 0u;

    {
        size_t i;
        if (!( i = count() ))
            return false;
        if (!check())
            return false;
        if (i == B)
            return true;
    }

    fail = false;
    {
        unsigned char a;
        size_t b, c, d, e, f;
        bool success;
        do {
            success = false;
            for (a = 0u; a < 3u; ++a)
            for (b = 0u; b <  A; ++b)
            for (c = 0u; c <  A; ++c)
                if (!content[ d = position[a][b][c] ]) {
                    for (e = 0u; e < A; ++e)
                        if (
                            e != c
                         && content[ f = position[a][b][e] ]
                         && modNumber(d, content[f])
                        ) {
                            if (SudokuDone())
                                goto SudokuInterrupt;
                            success = true;
                        }
                    for (e = 1u; e <= A; ++e)
                        if (Possibility(d, e)) {
                            for (f = 0u; f < A; ++f)
                                if (f != c && Possibility(position[a][b][f], e))
                                    goto ContinueParentLoop;
                            number(d, e);
                            if (SudokuDone())
                                goto SudokuInterrupt;
                            success = true;
                            break;
                            ContinueParentLoop:;
                        }
                }
            if (X)
                for (a = 0u; a < 2u; ++a)
                for (c = 0u; c <  A; ++c)
                    if (!content[ d = XPosition[a][c] ]) {
                        for (e = 0u; e < A; ++e)
                            if (
                                e != c
                             && content[ f = XPosition[a][e] ]
                             && modNumber(d, content[f])
                            ) {
                                if (SudokuDone())
                                    goto SudokuInterrupt;
                                success = true;
                            }
                        for (e = 1u; e <= A; ++e)
                            if (Possibility(d, e)) {
                                for (f = 0u; f < A; ++f)
                                    if (f != c && Possibility(XPosition[a][f], e))
                                        goto ContinueParentLoopX;
                                number(d, e);
                                if (SudokuDone())
                                    goto SudokuInterrupt;
                                success = true;
                                break;
                                ContinueParentLoopX:;
                            }
                    }
        } while (success);
    }

    for (size_t i = 0u; i < B; ++i)
        content[i] = 0u;
    for (size_t n = 0u;;) {
        while (++content[n] <= A)
            if (Possibility(n, content[n]) && test(n)) {
                if (n < B - 1u)
                    ++n;
                else goto SudokuInterrupt;
            }
        if (n)
            content[n--] = 0u;
        else return false;
    }

    SudokuInterrupt:
    if (!fail)
        fail = !check();
    return !fail;
}

#undef SudokuDone

bool NumberPlacer::modNumber(size_t n, size_t x) {
    bool b = possibilities[n][--x];
    if (b) {
        possibilities[n][x] = false;
        auto i = count(n);
        if (i) {
            if (i == 1u) {
                size_t j = 1u;
                while (!Possibility(n, j))
                    ++j;
                content[n] = j;
            }
        } else fail = true;
    }
    return b;
}

#undef Possibility

NumberPlacer& NumberPlacer::operator >> (size_t& x) {
    x = content[output++];
    return *this;
}

void NumberPlacer::number(size_t n, size_t x) {
    content[n] = x;
    for (size_t i = 0u; i < A; ++i)
        possibilities[n][i] = x ? (i == x - 1u) : true;
}

size_t NumberPlacer::count(void) {
    size_t x = 0u;
    for (size_t i = 0u; i < B; ++i)
        if (content[i])
            ++x;
    return x;
}

bool NumberPlacer::check() {
    unsigned char i;
    size_t j, k, l, m;
    for (i = 0u; i <     3u; ++i)
    for (j = 0u; j < A     ; ++j)
    for (k = 0u; k < A - 1u; ++k)
        if (content[ l = position[i][j][k] ])
            for (m = k + 1u; m < A; ++m)
                if (content[l] == content[ position[i][j][m] ])
                    return false;
    if (X)
        for (i = 0u; i <     2u; ++i)
        for (k = 0u; k < A - 1u; ++k)
            if (content[ l = XPosition[i][k] ])
                for (m = k + 1u; m < A; ++m)
                    if (content[l] == content[ XPosition[i][m] ])
                        return false;
    return true;
}

unsigned char NumberPlacer::count(size_t n) {
    unsigned char x = 0u;
    for (size_t i = 0u; i < A; ++i)
        if (possibilities[n][i] && ++x > 1u)
            break;
    return x;
}

bool NumberPlacer::test(size_t n) {
    unsigned char a;
    size_t b, c, d;
    for (a = 0u; a < 3u; ++a)
    for (b = 0u; b <  A; ++b)
        if (b != address[a][1][n]) {
            c = position[a][ address[a][0][n] ][b];
            if (content[c] &&
                content[c] == content[n])
                return false;
        }
    if (X)
        for (a = 0u; a < 2u; ++a)
            if (XAddress(n, a, b))
                for (c = 0u; c < A; ++c)
                    if (c != b) {
                        d = XPosition[a][c];
                        if (content[d] &&
                            content[d] == content[n])
                            return false;
                    }
    return true;
}

bool NumberPlacer::XAddress(size_t x, unsigned char a, size_t& b) {
    if (!XAddressValid[a][x])
        return false;
    b = a ? (x / (A - 1u) - 1u)
          : (x / (A + 1u));
    return true;
}

void NumberPlacer::dealloc(void) {
    dealloc(possibilities, B);
    dealloc(content);
    {
        unsigned char i, j;
        for (i = 0u; i < 2u; ++i) {
            dealloc(XAddressValid[i]);
            dealloc(XPosition[i]);
        }
        for (i = 0u; i < 3u; ++i) {
            for (j = 0u; j < 2u; ++j)
                dealloc(address[i][j]);
            dealloc(position[i], A);
        }
    }
}

template<class T>
void NumberPlacer::dealloc(T p) {
    if (p != nullptr)
        delete[] p;
}

template<class T>
void NumberPlacer::dealloc(T p, size_t s) {
    if (p != nullptr) {
        for (size_t i = 0u; i < s; ++i)
            dealloc(p[i]);
        delete[] p;
    }
}
// ~NumberPlacer
