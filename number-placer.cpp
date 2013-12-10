/* Al Klimov's Number Placer
 * Copyright (C) 2013 Alexander A. Klimov
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
#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
using namespace std;

unsigned long long sudokuSize[4], sudokuStrSize[2], sudokuInStrLen;
bool sudokuX = false, firstLine = true, sudokuFail, sudokuSuccess, sudokuTestReady;
vector<unsigned long long> sudokuTestCache, sudokuTestContent;
vector< vector<bool> > sudokuContent;
vector< vector< vector<unsigned long long> > > sudokuPosition (3), sudokuAddress (3);
vector< vector<unsigned long long> > sudokuXPosition (2);
string sudokuInStr;

void invalid_cmd_arg (char* c, int i, string s)
{
	cerr << "Error: Invalid command-line argument (" << i << "): '" << c << "'" << endl
		<< "Must be " << s << "!" << endl;
}

bool cstr_to_uint (char* c, unsigned long long& i)
{
	unsigned long long s = strlen(c);
	if (s == 0 || (s > 1 && c[0] == 48))
		return false;
	unsigned long long j, k, l, m, x = 0;
	for (j = 0; j < s; j++)
	{
		k = s - 1 - j;
		if (48 <= c[k] && c[k] <= 57)
		{
			l = c[k] - 48;
			if (l != 0)
			{
				for (m = 0; m < j; m++)
					l *= 10;
				x += l;
			}
		}
		else
			return false;
	}
	i = x;
	return true;
}

vector<unsigned long long> uint_division (unsigned long long a, unsigned long long b)
{
	vector<unsigned long long> c (2);
	c[0] = 0;
	while (a >= b)
	{
		a -= b;
		c[0] += 1;
	}
	c[1] = a;
	return c;
}

unsigned long long getInStr()
{
	getline(cin, sudokuInStr);
	return sudokuInStr.length();
}

void setNumber (unsigned long long n, unsigned long long x)
{
	for (unsigned long long i = 0; i < sudokuSize[2]; i++)
		sudokuContent[n][i] = (x == 0) ? true : (i == x - 1);
}

unsigned long long sudokuCount (unsigned long long n)
{
	unsigned long long x = 0;
	for (unsigned long long i = 0; i < sudokuSize[2]; i++)
		if (sudokuContent[n][i])
			x++;
	return x;
}

unsigned long long sudokuCountAll()
{
	unsigned long long x = 0;
	for (unsigned long long i = 0; i < sudokuSize[3]; i++)
		if (sudokuCount(i) == 1)
			x++;
	return x;
}

unsigned long long getNumber (unsigned long long n)
{
	unsigned long long i = 0;
	if (sudokuCount(n) == 1)
	{
		while (! sudokuContent[n][i])
			i++;
		i++;
	}
	return i;
}

unsigned long long uint_digits (unsigned long long n)
{
	if (n == 0)
		return 1;
	unsigned long long x = 0;
	for (unsigned long long i = 1; i <= n; i *= 10)
		x++;
	return x;
}

void sudokuPrint (bool b = false)
{
	for (unsigned long long i = 0, j, k; i < sudokuSize[3]; i++)
	{
		j = b ? getNumber(i) : 0;
		for (k = uint_digits(j); k < sudokuStrSize[0]; k++)
			cout << 0;
		cout << j;
	}
	cout << endl;
}

bool sudokuCheck()
{
	vector<unsigned long long> x (sudokuSize[3]);
	for (unsigned long long i = 0; i < sudokuSize[3]; i++)
		x[i] = getNumber(i);
	unsigned char i;
	unsigned long long j, k, l;
	for (i = 0; i < 3; i++)
		for (j = 0; j < sudokuSize[2]; j++)
			for (k = 0; k < sudokuSize[2] - 1; k++)
				if (x[sudokuPosition[i][j][k]] != 0)
					for (l = k + 1; l < sudokuSize[2]; l++)
						if (x[sudokuPosition[i][j][k]] == x[sudokuPosition[i][j][l]])
							return false;
	if (sudokuX)
		for (i = 0; i < 2; i++)
			for (k = 0; k < sudokuSize[2] - 1; k++)
				if (x[sudokuXPosition[i][k]] != 0)
					for (l = k + 1; l < sudokuSize[2]; l++)
						if (x[sudokuXPosition[i][k]] == x[sudokuXPosition[i][l]])
							return false;
	return true;
}

bool sudokuDone()
{
	return sudokuFail || sudokuCountAll() == sudokuSize[3];
}

bool modNumber (unsigned long long n, unsigned long long x, bool r = false)
{
	x--;
	bool b = sudokuContent[n][x];
	if (r && b)
		sudokuContent[n][x] = false;
	return b;
}

bool sudokuXAddress (unsigned long long x, unsigned char a, unsigned long long& b)
{
	for (unsigned long long i = 0; i < sudokuSize[2]; i++)
		if (sudokuXPosition[a][i] == x)
		{
			b = i;
			return true;
		}
	return false;
}

void sudokuTest (unsigned long long n = 0)
{
	if (n == 0)
	{
		sudokuTestReady = false;
		for (unsigned long long i = 0; i < sudokuSize[3]; i++)
			sudokuTestContent[i] = 0;
	}
	unsigned char a;
	unsigned long long b, c, d, e;
	bool x;
	for (sudokuTestCache[n] = 1; sudokuTestCache[n] <= sudokuSize[2] && ! sudokuTestReady; sudokuTestCache[n]++)
		if (modNumber(n, sudokuTestCache[n]))
		{
			x = true;
			sudokuTestContent[n] = sudokuTestCache[n];
			for (a = 0; a < 3 && x; a++)
			{
				b = sudokuAddress[a][n][0];
				c = sudokuAddress[a][n][1];
				for (d = 0; d < sudokuSize[2] && x; d++)
					if (d != c)
					{
						e = sudokuPosition[a][b][d];
						if (sudokuTestContent[e] != 0 && sudokuTestContent[e] == sudokuTestContent[n])
							x = false;
					}
			}
			if (sudokuX && x)
				for (a = 0; a < 2 && x; a++)
					if (sudokuXAddress(n, a, b))
						for (c = 0; c < sudokuSize[2] && x; c++)
							if (c != b)
							{
								d = sudokuXPosition[a][c];
								if (sudokuTestContent[d] != 0 && sudokuTestContent[d] == sudokuTestContent[n])
									x = false;
							}
			if (x)
			{
				if (n < sudokuSize[3] - 1)
				{
					sudokuTest(n + 1);
					sudokuTestContent[n + 1] = 0;
				}
				else
				{
					sudokuTestReady = true;
					for (unsigned long long i = 0; i < sudokuSize[3]; i++)
						setNumber(i, sudokuTestContent[i]);
				}
			}
		}
	if (n == 0 && ! sudokuTestReady)
		sudokuFail = true;
}

int main (int argc, char** argv)
{
	cerr << "Al Klimov's Number Placer" << endl
		<< "Copyright (C) 2013 Alexander A. Klimov" << endl
		<< endl;
	if (argc > 4)
	{
		cerr << "Error: '" << argv[0] << "' takes at most 3 command-line arguments (" << (argc - 1) << " given)" << endl;
		return EXIT_FAILURE;
	}
	if (argc > 1)
	{
		if (strlen(argv[argc - 1]) == 1 && (argv[argc - 1][0] == 88 || argv[argc - 1][0] == 120))
			sudokuX = true;
		else if (argc == 4)
		{
			invalid_cmd_arg(argv[3], 3, "'X' or 'x'");
			return EXIT_FAILURE;
		}
	}
	for (unsigned char j = 0; j < 2; j++)
	{
		if (argc - (sudokuX ? 2 : 1) < j + 1)
			sudokuSize[j] = (j == 0) ? 3 : sudokuSize[0];
		else if (! cstr_to_uint(argv[j + 1], sudokuSize[j]) || sudokuSize[j] < 2)
		{
			invalid_cmd_arg(argv[j + 1], j + 1, "an integer >= 2");
			return EXIT_FAILURE;
		}
	}
	sudokuSize[2] = sudokuSize[0] * sudokuSize[1];
	cerr << "Sudoku size: " << sudokuSize[0] << "x" << sudokuSize[1] << " (" << sudokuSize[2] << "x" << sudokuSize[2] << ")" << endl
		<< "X-Sudoku: " << (sudokuX ? "yes" : "no") << endl
		<< endl
		<< "Preparing...";
	sudokuSize[3] = sudokuSize[2] * sudokuSize[2];
	sudokuContent.resize(sudokuSize[3]);
	sudokuTestCache.resize(sudokuSize[3]);
	sudokuTestContent.resize(sudokuSize[3]);
	for (unsigned long long i = 0; i < sudokuSize[3]; i++)
		sudokuContent[i].resize(sudokuSize[2]);
	for (unsigned char i = 0; i < 3; i++)
	{
		sudokuAddress[i].resize(sudokuSize[3]);
		for (unsigned long long j = 0; j < sudokuSize[3]; j++)
			sudokuAddress[i][j].resize(2);
		sudokuPosition[i].resize(sudokuSize[2]);
		for (unsigned long long j = 0, k; j < sudokuSize[2]; j++)
		{
			sudokuPosition[i][j].resize(sudokuSize[2]);
			for (k = 0; k < sudokuSize[2]; k++)
			{
				if (i == 2)
				{
					vector<unsigned long long>
						l = uint_division(j, sudokuSize[1]),
						m = uint_division(k, sudokuSize[0]);
					sudokuPosition[i][j][k] =
						l[0] * sudokuSize[2] * sudokuSize[1] +
						m[0] * sudokuSize[2] +
						l[1] * sudokuSize[0] +
						m[1];
				}
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
			for (unsigned long long j = 0; j < sudokuSize[2]; j++)
				sudokuXPosition[i][j] = j * sudokuSize[2] + ((i == 0) ? j : (sudokuSize[2] - 1 - j));
		}
	sudokuStrSize[0] = uint_digits(sudokuSize[2]);
	sudokuStrSize[1] = sudokuStrSize[0] * sudokuSize[3];
	cerr << " done." << endl;
	while (true)
	{
		sudokuInStrLen = getInStr();
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
			for (unsigned long long i = 0; i < sudokuStrSize[1]; i++)
				if (! (48 <= sudokuInStr[i] && sudokuInStr[i] <= 57))
				{
					cerr << "Error: Invalid input! Each character must be a decimal number!" << endl;
					return EXIT_FAILURE;
				}
			for (unsigned long long i = 0, j, k, l, m; i < sudokuSize[3]; i++)
			{
				j = 0;
				for (k = 0; k < sudokuStrSize[0]; k++)
				{
					l = sudokuInStr[i * sudokuStrSize[0] + sudokuStrSize[0] - 1 - k] - 48;
					if (l != 0)
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
			unsigned long long i = sudokuCountAll();
			if (i == 0)
				sudokuPrint();
			else if (sudokuCheck())
			{
				if (i == sudokuSize[3])
					sudokuPrint(true);
				else
				{
					sudokuFail = false;
					sudokuSuccess = true;
					unsigned char a;
					unsigned long long b, c, d, e, f, g;
					bool x, y;
					while (sudokuSuccess && ! sudokuDone())
					{
						sudokuSuccess = false;
						for (a = 0; a < 3             && ! sudokuDone(); a++)
						for (b = 0; b < sudokuSize[2] && ! sudokuDone(); b++)
						for (c = 0; c < sudokuSize[2] && ! sudokuDone(); c++)
						{
							d = sudokuPosition[a][b][c];
							if (sudokuCount(d) > 1)
							{
								for (e = 0; e < sudokuSize[2] && ! sudokuDone(); e++)
									if (e != c)
									{
										f = sudokuPosition[a][b][e];
										if (sudokuCount(f) == 1)
											if (modNumber(d, getNumber(f), true))
											{
												sudokuSuccess = true;
												if (sudokuCount(d) == 0)
													sudokuFail = true;
											}
									}
								if (! sudokuDone())
								{
									x = true;
									for (e = 1; e <= sudokuSize[2] && x; e++)
										if (modNumber(d, e))
										{
											y = true;
											for (f = 0; f < sudokuSize[2] && x && y; f++)
												if (f != c)
												{
													g = sudokuPosition[a][b][f];
													if (modNumber(g, e))
														y = false;
												}
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
						if (sudokuX && ! sudokuDone())
							for (a = 0; a < 2             && ! sudokuDone(); a++)
							for (c = 0; c < sudokuSize[2] && ! sudokuDone(); c++)
							{
								d = sudokuXPosition[a][c];
								if (sudokuCount(d) > 1)
								{
									for (e = 0; e < sudokuSize[2] && ! sudokuDone(); e++)
										if (e != c)
										{
											f = sudokuXPosition[a][e];
											if (sudokuCount(f) == 1)
												if (modNumber(d, getNumber(f), true))
												{
													sudokuSuccess = true;
													if (sudokuCount(d) == 0)
														sudokuFail = true;
												}
										}
									if (! sudokuDone())
									{
										x = true;
										for (e = 1; e <= sudokuSize[2] && x; e++)
											if (modNumber(d, e))
											{
												y = true;
												for (f = 0; f < sudokuSize[2] && x && y; f++)
													if (f != c)
													{
														g = sudokuXPosition[a][f];
														if (modNumber(g, e))
															y = false;
													}
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
					if (! sudokuDone())
						sudokuTest();
					if (! sudokuFail)
						sudokuFail = ! sudokuCheck();
					sudokuPrint(! sudokuFail);
				}
			}
			else
				sudokuPrint();
			if (cin.eof())
			{
				cerr << "End of file." << endl
					<< "Warning: No EOL @ EOF" << endl;
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
