#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>

namespace puzzle
{
    using namespace std;
    using sudoku_t = vector<string>;

    const int gSudokuSize = 9;
    const int gSudokuCellSize = 3;
    // the remain blanks
    const int gGenerateMinCount = 11;
    const int gMinLevel = 9;
    const int gMaxLevel = 64; // 81 - 17;
    const int gDefaultLevel = 30;

    class Sudoku
    {
    public:
        Sudoku() : Sudoku(gDefaultLevel) {}
        Sudoku(const int level) : mLevel(level)
        {
            mValueList.clear();
            for (char c = '1'; c <= '0' + mSize; c++)
                mValueList.emplace_back(c);
        }
        Sudoku(Sudoku &&) = default;
        Sudoku(const Sudoku &) = default;

        Sudoku &operator=(Sudoku &&other) noexcept
        {
            return this->operator=(static_cast<const Sudoku &>(other));
        }
        Sudoku &operator=(const Sudoku &other) noexcept
        {
            this->mSudoku = other.mSudoku;
            this->mSudokuSolved = other.mSudokuSolved;
            this->mLevel = other.mLevel;
            return *this;
        }

        void generate()
        {
            srand(static_cast<unsigned int>(time(nullptr)));
            random_shuffle(mValueList.begin(), mValueList.end());
            do
            {
                resetSudoku();
                generateMiniSudoku();
            } while (!isFitRules(mSudokuSolved) || !solve(mSudokuSolved));
            generatePuzzle();
        }

        void generate(const int level, const bool singleSolution)
        {
            mLevel = level;
            mSingleSolution = singleSolution;
            generate();
        }

        void generate(const int level)
        {
            generate(level, mSingleSolution);
        }

        void generate(const bool singleSolution)
        {
            generate(mLevel, singleSolution);
        }

        sudoku_t &data()
        {
            return mSudoku;
        }

        sudoku_t &dataSolve()
        {
            return mSudokuSolved;
        }

        bool solve()
        {
            mSudokuSolved = mSudoku;
            return isFitRules(mSudokuSolved) && solve(mSudokuSolved);
        }

        int size() const
        {
            return mSize;
        }

        int cellSize() const
        {
            return mCellSize;
        }

        char value(int y, int x) const
        {
            return mSudoku[y][x];
        }

        bool inRange(char val) const
        {
            return '1' <= val && val <= ('0' + mSize);
        }

        sudoku_t::value_type &operator[](int y)
        {
            return mSudoku[y];
        }

        bool check()
        {
            return isFitRules(mSudoku);
        }

        bool check(sudoku_t &sudoku)
        {
            return isFitRules(sudoku);
        }

        operator bool() const
        {
            return !mSudoku.empty();
        }

        static char blank()
        {
            return '.';
        }

        bool canSetValue(int i, int j, char val) const
        {
            return canSetValue(mSudoku, i, j, val);
        }

    private:
        void resetSudoku()
        {
            mSudokuSolved.resize(mSize);
            for (int i = 0; i < mSize; i++)
                mSudokuSolved[i] = string(mSize, blank());
        }

        void generateMiniSudoku()
        {
            srand(static_cast<unsigned int>(time(nullptr)));

            int minCount = gGenerateMinCount;
            int i = 0, j = 0; // y, x
            char val = blank();

            while (minCount-- > 0)
            {
                do
                {
                    i = rand() % mSize;
                    j = rand() % mSize;
                    val = rand() % mSize + '1';
                } while (!canSetValue(mSudokuSolved, i, j, val));
                mSudokuSolved[i][j] = val;
            }
        }

        void generatePuzzle()
        {
            int blankCount = mLevel;
            int i = 0, j = 0; // y, x

            mSudoku = mSudokuSolved;
            sudoku_t singleSulutionSolved = mSudoku;
            while (blankCount-- > 0)
            {
                srand(static_cast<unsigned int>(time(nullptr)));
                do
                {
                    do
                    {
                        i = rand() % mSize;
                        j = rand() % mSize;
                    } while (mSudoku[i][j] == blank());
                    mSudoku[i][j] = blank();
                    singleSulutionSolved[i][j] = blank();
                } while (mSingleSolution && isSingleSolution(singleSulutionSolved));
            }
        }

        bool canSetValue(const sudoku_t &sudoku, int i, int j, char val) const
        {
            for (int k = 0; k < mSize; k++)
                if (sudoku[i][k] == val)
                    return false;
            for (int k = 0; k < mSize; k++)
                if (sudoku[k][j] == val)
                    return false;

            int cell_i = (i / mCellSize) * mCellSize;
            int cell_j = (j / mCellSize) * mCellSize;
            for (int k = cell_i; k < cell_i + mCellSize; k++)
            {
                for (int l = cell_j; l < cell_j + mCellSize; l++)
                {
                    if (sudoku[k][l] == val)
                        return false;
                }
            }

            return true;
        }

        bool isFitRules(sudoku_t &sudoku)
        {
            for (int y = 0; y < mSize; y++)
            {
                for (int x = 0; x < mSize; x++)
                {
                    if (sudoku[y][x] == blank())
                        continue;

                    char originalVal = sudoku[y][x];
                    sudoku[y][x] = blank();
                    if (!canSetValue(sudoku, y, x, originalVal))
                    {
                        sudoku[y][x] = originalVal;
                        return false;
                    }
                    sudoku[y][x] = originalVal;
                }
            }
            return true;
        }

        bool isSingleSolution(sudoku_t &sudoku, int count = 0)
        {
            for (int y = 0; y < mSize; y++)
            {
                for (int x = 0; x < mSize; x++)
                {
                    if (sudoku[y][x] != blank())
                        continue;

                    for (char c : mValueList)
                    {
                        if (!canSetValue(sudoku, y, x, c))
                            continue;

                        sudoku[y][x] = c;
                        if (isSingleSolution(sudoku, count))
                            count++;
                        if (count > 1)
                            return false;
                        sudoku[y][x] = blank();
                    }

                    return false;
                }
            }
            return true;
        }

        bool solve(sudoku_t &sudoku)
        {
            for (int y = 0; y < mSize; y++)
            {
                for (int x = 0; x < mSize; x++)
                {
                    if (sudoku[y][x] != blank())
                        continue;

                    for (char c : mValueList)
                    {
                        if (!canSetValue(sudoku, y, x, c))
                            continue;

                        sudoku[y][x] = c;
                        if (solve(sudoku))
                            return true;
                        sudoku[y][x] = blank();
                    }

                    return false;
                }
            }
            return true;
        }

    private:
        sudoku_t mSudoku;
        sudoku_t mSudokuSolved;
        const int mSize = gSudokuSize;
        const int mCellSize = gSudokuCellSize;
        int mLevel = gDefaultLevel;
        vector<char> mValueList;
        bool mSingleSolution = false;
    };
};
