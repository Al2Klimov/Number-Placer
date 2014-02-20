/* Al Klimov's Number Placer  1.0.12 (2014-02-20)
 * Copyright (C) 2013-2014  Alexander A. Klimov
 * Powered by C++11
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
#include <cstring> // strlen
#include <string>  // string, getline
#include <vector>
#include <array>
#include <cstdint> // uintmax_t
#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
using namespace std;

char** _argv;
bool sudokuX = false,
     sudokuFail,
     sudokuTestReady;
uintmax_t sudokuSize[4],
          sudokuStrSize[2];
vector<uintmax_t> sudokuContent,
                  sudokuTestContent,
                  sudokuXPosition[2];
vector<vector<bool>> sudokuPossibilities;
vector<vector<uintmax_t>> sudokuPosition[3];
vector<array<uintmax_t,2>> sudokuAddress[3];

inline void invalid_cmd_arg(int, string);
inline bool getNumberPossibility(uintmax_t, uintmax_t);
inline bool sudokuDone();
void setNumber(uintmax_t, uintmax_t);
void sudokuPrint(bool);
void sudokuTest(uintmax_t);
void sudokuTest();
bool sudokuCheck();
bool modNumber(uintmax_t, uintmax_t);
bool sudokuXAddress(uintmax_t, unsigned char, uintmax_t&);
uintmax_t sudokuCount(uintmax_t);
uintmax_t sudokuCount();
uintmax_t uint_digits(uintmax_t);

int main(int argc, char** argv)
{
    cerr << "Al Klimov's Number Placer  1.0.12\n"
            "Copyright (C) 2013-2014  Alexander A. Klimov\n" << endl;
    _argv = argv;
    if (argc > 4)
    {
        cerr << "Error: '" << argv[0] << "' takes at most 3 command-line arguments (" << (argc - 1) << " given)" << endl;
        return EXIT_FAILURE;
    }
    if (argc > 1)
    {
        if (strlen(argv[argc-1]) == 1 && (argv[argc-1][0] == 88 || argv[argc-1][0] == 120))
            sudokuX = true;
        else if (argc == 4)
        {
            invalid_cmd_arg(3, "'X' or 'x'");
            return EXIT_FAILURE;
        }
    }
    for (signed char i = 0; i < 2; i++)
    {
        if (argc - (sudokuX ? 2 : 1) < i + 1)
            sudokuSize[i] = (i == 0) ? 3 : sudokuSize[0];
        else
        {
            auto s = strlen(argv[i+1]);
            bool b = !(s == 0 || (s > 1 && argv[i+1][0] == 48));
            if (b)
            {
                uintmax_t l, x = 0;
                for (decltype(s) j = 0, k = s, m; j < s; j++)
                {
                    k--;
                    if (48 <= argv[i+1][k] && argv[i+1][k] <= 57)
                    {
                        if ((l = argv[i+1][k] - 48) != 0)
                        {
                            for (m = 0; m < j; m++)
                                l *= 10;
                            x += l;
                        }
                    }
                    else
                    {
                        b = false;
                        break;
                    }
                }
                if (b)
                    sudokuSize[i] = x;
            }
            if (!b || sudokuSize[i] < 2)
            {
                invalid_cmd_arg(i + 1,
                    (!sudokuX && argc - 1 == i + 1)
                    ? "an integer >= 2 or 'X' or 'x'"
                    : "an integer >= 2");
                return EXIT_FAILURE;
            }
        }
    }
    sudokuSize[2] = sudokuSize[0] * sudokuSize[1];
    cerr << "Sudoku size: " << sudokuSize[0] << "x" << sudokuSize[1] << " (" << sudokuSize[2] << "x" << sudokuSize[2] << ")\n"
            "X-Sudoku: " << (sudokuX ? "yes" : "no") << "\n"
            "\n"
            "Preparing...";
    sudokuPossibilities.resize(sudokuSize[3] = sudokuSize[2] * sudokuSize[2]);
    sudokuContent.resize(sudokuSize[3]);
    sudokuTestContent.resize(sudokuSize[3]);
    for (uintmax_t i = 0; i < sudokuSize[3]; i++)
        sudokuPossibilities[i].resize(sudokuSize[2]);
    for (unsigned char i = 0; i < 3; i++)
    {
        sudokuAddress[i].resize(sudokuSize[3]);
        sudokuPosition[i].resize(sudokuSize[2]);
        for (uintmax_t j = 0, k; j < sudokuSize[2]; j++)
        {
            sudokuPosition[i][j].resize(sudokuSize[2]);
            for (k = 0; k < sudokuSize[2]; k++)
            {
                if (i == 2)
                    sudokuPosition[2][j][k] =
                        j / sudokuSize[1] * sudokuSize[2] * sudokuSize[1] +
                        k / sudokuSize[0] * sudokuSize[2] +
                        j % sudokuSize[1] * sudokuSize[0] +
                        k % sudokuSize[0];
                else
                    sudokuPosition[i][j][k] = (i == 0) ? (j * sudokuSize[2] + k) : (k * sudokuSize[2] + j);
                sudokuAddress[i][sudokuPosition[i][j][k]][0] = j;
                sudokuAddress[i][sudokuPosition[i][j][k]][1] = k;
            }
        }
    }
    if (sudokuX)
        for (unsigned char i = 0; i < 2; i++)
        {
            sudokuXPosition[i].resize(sudokuSize[2]);
            for (uintmax_t j = 0; j < sudokuSize[2]; j++)
                sudokuXPosition[i][j] = j * sudokuSize[2] + ((i == 0) ? j : (sudokuSize[2] - 1 - j));
        }
    sudokuStrSize[1] = (sudokuStrSize[0] = uint_digits(sudokuSize[2])) * sudokuSize[3];
    bool firstLine = true, sudokuSuccess;
    string sudokuInStr;
    cerr << " done." << endl;
    for (;;)
    {
        getline(cin, sudokuInStr);
        auto sudokuInStrLen = sudokuInStr.length();
        if (sudokuInStrLen == 0)
        {
            if (cin.eof())
            {
                cerr << (firstLine ? "EOF @ first line -- nothing to do." : "End of file.") << endl;
                return EXIT_SUCCESS;
            }
            else
            {
                cerr << "Warning: Ignoring blank line" << endl;
                if (firstLine)
                    firstLine = false;
            }
        }
        else if (sudokuInStrLen == sudokuStrSize[1])
        {
            for (uintmax_t i = 0; i < sudokuStrSize[1]; i++)
                if (!(48 <= sudokuInStr[i] && sudokuInStr[i] <= 57))
                {
                    cerr << "Error: Invalid input! Each character must be a decimal number!" << endl;
                    return EXIT_FAILURE;
                }
            for (uintmax_t i = 0, j, k, l, m; i < sudokuSize[3]; i++)
            {
                j = 0;
                for (k = 0; k < sudokuStrSize[0]; k++)
                {
                    if ((l = sudokuInStr[(i + 1) * sudokuStrSize[0] - 1 - k] - 48) != 0)
                    {
                        for (m = 0; m < k; m++)
                            l *= 10;
                        j += l;
                    }
                }
                if (j <= sudokuSize[2])
                    setNumber(i, j);
                else
                {
                    cerr << "Error: Invalid input! Number (" << j << ") out of range (0-" << sudokuSize[2] << ")!" << endl;
                    return EXIT_FAILURE;
                }
            }
            auto i = sudokuCount();
            if (i == 0)
                sudokuPrint(false);
            else if (sudokuCheck())
            {
                if (i == sudokuSize[3])
                    sudokuPrint(true);
                else
                {
                    sudokuFail = false;
                    sudokuSuccess = true;
                    unsigned char a;
                    uintmax_t b, c, d, e, f;
                    bool x, y;
                    while (sudokuSuccess && !sudokuDone())
                    {
                        sudokuSuccess = false;
                        for (a = 0; a <             3 && !sudokuDone(); a++)
                        for (b = 0; b < sudokuSize[2] && !sudokuDone(); b++)
                        for (c = 0; c < sudokuSize[2] && !sudokuDone(); c++)
                        {
                            if (sudokuCount(d = sudokuPosition[a][b][c]) > 1)
                            {
                                for (e = 0; e < sudokuSize[2] && !sudokuDone(); e++)
                                    if (e != c)
                                    {
                                        if (sudokuCount(f = sudokuPosition[a][b][e]) == 1)
                                            if (modNumber(d, sudokuContent[f]))
                                                sudokuSuccess = true;
                                    }
                                if (!sudokuDone())
                                {
                                    x = true;
                                    for (e = 1; e <= sudokuSize[2] && x; e++)
                                        if (getNumberPossibility(d, e))
                                        {
                                            y = true;
                                            for (f = 0; f < sudokuSize[2] && x && y; f++)
                                                if (f != c && getNumberPossibility(sudokuPosition[a][b][f], e))
                                                    y = false;
                                            if (y)
                                            {
                                                x = false;
                                                setNumber(d, e);
                                                sudokuSuccess = true;
                                            }
                                        }
                                }
                            }
                        }
                        if (sudokuX && !sudokuDone())
                            for (a = 0; a <             2 && !sudokuDone(); a++)
                            for (c = 0; c < sudokuSize[2] && !sudokuDone(); c++)
                            {
                                if (sudokuCount(d = sudokuXPosition[a][c]) > 1)
                                {
                                    for (e = 0; e < sudokuSize[2] && !sudokuDone(); e++)
                                        if (e != c)
                                        {
                                            if (sudokuCount(f = sudokuXPosition[a][e]) == 1)
                                                if (modNumber(d, sudokuContent[f]))
                                                    sudokuSuccess = true;
                                        }
                                    if (!sudokuDone())
                                    {
                                        x = true;
                                        for (e = 1; e <= sudokuSize[2] && x; e++)
                                            if (getNumberPossibility(d, e))
                                            {
                                                y = true;
                                                for (f = 0; f < sudokuSize[2] && x && y; f++)
                                                    if (f != c && getNumberPossibility(sudokuXPosition[a][f], e))
                                                        y = false;
                                                if (y)
                                                {
                                                    x = false;
                                                    setNumber(d, e);
                                                    sudokuSuccess = true;
                                                }
                                            }
                                    }
                                }
                            }
                    }
                    if (!sudokuDone())
                        sudokuTest();
                    if (!sudokuFail)
                        sudokuFail = !sudokuCheck();
                    sudokuPrint(!sudokuFail);
                }
            }
            else
                sudokuPrint(false);
            if (cin.eof())
            {
                cerr << "End of file.\n"
                        "Warning: No EOL @ EOF" << endl;
                return EXIT_SUCCESS;
            }
            if (firstLine)
                firstLine = false;
        }
        else
        {
            cerr << "Error: Invalid input! Each line must be " << sudokuStrSize[1] << " characters long!" << endl;
            return EXIT_FAILURE;
        }
    }
}

void invalid_cmd_arg(int i, string s)
{
    cerr << "Error: Invalid command-line argument (" << i << "): '" << _argv[i] << "'\n"
            "Must be " << s << "!" << endl;
}

void setNumber(uintmax_t n, uintmax_t x)
{
    sudokuContent[n] = x;
    for (uintmax_t i = 0; i < sudokuSize[2]; i++)
        sudokuPossibilities[n][i] = (x == 0) ? true : (i == x - 1);
}

uintmax_t sudokuCount(uintmax_t n)
{
    uintmax_t x = 0;
    for (uintmax_t i = 0; i < sudokuSize[2]; i++)
        if (sudokuPossibilities[n][i])
            x++;
    return x;
}

uintmax_t sudokuCount()
{
    uintmax_t x = 0;
    for (uintmax_t i = 0; i < sudokuSize[3]; i++)
        if (sudokuCount(i) == 1)
            x++;
    return x;
}

bool getNumberPossibility(uintmax_t n, uintmax_t x)
{
    return sudokuPossibilities[n][x-1];
}

uintmax_t uint_digits(uintmax_t n)
{
    if (n == 0)
        return 1;
    uintmax_t x = 0;
    for (uintmax_t i = 1; i <= n; i *= 10)
        x++;
    return x;
}

void sudokuPrint(bool b)
{
    if (b)
        for (uintmax_t i = 0; i < sudokuSize[3]; i++)
        {
            for (auto k = uint_digits(sudokuContent[i]); k < sudokuStrSize[0]; k++)
                cout << 0;
            cout << sudokuContent[i];
        }
    else
        for (uintmax_t i = 0; i < sudokuSize[3]   ; i++)
        for (uintmax_t j = 0; j < sudokuStrSize[0]; j++)
            cout << 0;
    cout << endl;
}

bool sudokuCheck()
{
    unsigned char i;
    uintmax_t j, k, l, m;
    for (i = 0; i <                 3; i++)
    for (j = 0; j < sudokuSize[2]    ; j++)
    for (k = 0; k < sudokuSize[2] - 1; k++)
    {
        if (sudokuContent[l = sudokuPosition[i][j][k]] != 0)
            for (m = k + 1; m < sudokuSize[2]; m++)
                if (sudokuContent[l] == sudokuContent[sudokuPosition[i][j][m]])
                    return false;
    }
    if (sudokuX)
        for (i = 0; i <                 2; i++)
        for (k = 0; k < sudokuSize[2] - 1; k++)
        {
            if (sudokuContent[l = sudokuXPosition[i][k]] != 0)
                for (m = k + 1; m < sudokuSize[2]; m++)
                    if (sudokuContent[l] == sudokuContent[sudokuXPosition[i][m]])
                        return false;
        }
    return true;
}

bool sudokuDone()
{
    return sudokuFail || sudokuCount() == sudokuSize[3];
}

bool modNumber(uintmax_t n, uintmax_t x)
{
    bool b = sudokuPossibilities[n][--x];
    if (b)
        sudokuPossibilities[n][x] = false;
    auto i = sudokuCount(n);
    if (i == 0)
        sudokuFail = true;
    else if (i == 1)
    {
        uintmax_t j = 1;
        while (!getNumberPossibility(n, j))
            j++;
        sudokuContent[n] = j;
    }
    return b;
}

bool sudokuXAddress(uintmax_t x, unsigned char a, uintmax_t& b)
{
    for (uintmax_t i = 0; i < sudokuSize[2]; i++)
        if (sudokuXPosition[a][i] == x)
        {
            b = i;
            return true;
        }
    return false;
}

void sudokuTest()
{
    sudokuTestReady = false;
    for (uintmax_t i = 0; i < sudokuSize[3]; i++)
        sudokuTestContent[i] = 0;
    sudokuTest(0);
    if (!sudokuTestReady)
        sudokuFail = true;
}

void sudokuTest(uintmax_t n)
{
    unsigned char b;
    bool x;
    for (uintmax_t a = 1, c, d, e, f; a <= sudokuSize[2] && !sudokuTestReady; a++)
        if (getNumberPossibility(n, a))
        {
            x = true;
            sudokuTestContent[n] = a;
            for (b = 0; b < 3 && x; b++)
            {
                c = sudokuAddress[b][n][0];
                d = sudokuAddress[b][n][1];
                for (e = 0; e < sudokuSize[2] && x; e++)
                    if (e != d)
                    {
                        f = sudokuPosition[b][c][e];
                        if (sudokuTestContent[f] != 0 && sudokuTestContent[f] == sudokuTestContent[n])
                            x = false;
                    }
            }
            if (sudokuX && x)
                for (b = 0; b < 2 && x; b++)
                    if (sudokuXAddress(n, b, c))
                        for (d = 0; d < sudokuSize[2] && x; d++)
                            if (d != c)
                            {
                                e = sudokuXPosition[b][d];
                                if (sudokuTestContent[e] != 0 && sudokuTestContent[e] == sudokuTestContent[n])
                                    x = false;
                            }
            if (x)
            {
                if (n < sudokuSize[3] - 1)
                {
                    sudokuTest(n + 1);
                    sudokuTestContent[n+1] = 0;
                }
                else
                {
                    sudokuTestReady = true;
                    for (uintmax_t i = 0; i < sudokuSize[3]; i++)
                        setNumber(i, sudokuTestContent[i]);
                }
            }
        }
}
