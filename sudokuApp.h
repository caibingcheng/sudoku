#pragma once

#include <stdio.h>
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#endif

#include "sudoku.h"

#ifdef _WIN32
char getChar() {
	return _getch();
}
#endif

void gotoYx(int y, int x) {
	std::cout << ("\033[" + std::to_string(y) + ";" + std::to_string(x) + "H");
}

using namespace std;
using namespace puzzle;

const int gXBlankMargin = 2;
const int gYBlankMargin = 1;
const int gLevelMin = 1;
const int gLevelMax = 7;

const char* COLOR_FG_GREEN = "\033[1m\033[32m";
const char* COLOR_BG_UNDERLINE = "\033[4m";
const char* COLOR_BG_REVERSE = "\033[7m";
const char* COLOR_CLEAR = "\033[0m";

class SudokuApp {
private:
	enum class Move {
		UP,
		DOWN,
		LEFT,
		RIGHT,
	};

	struct Pos {
		int y;
		int x;

		bool operator== (const Pos& pos) const {
			return this->y == pos.y && this->x == pos.x;
		}

		bool operator!= (const Pos& pos) const {
			return !this->operator==(pos);
		}
	};
public:
	SudokuApp() : SudokuApp(1, 1) {}
	SudokuApp(int yMargin, int xMargin) : mXMargin(max(1, xMargin)), mYMargin(max(1, yMargin)) {
		initialize();
	}

	void run() {
		help();
		resetByLevel();
		gotoPos();

		char cmd = 0;
		do {
			cmd = getChar();
			clearInfo();

			// position change first
			if (cmd == 'w') move<Move::UP>();
			else if (cmd == 's') move<Move::DOWN>();
			else if (cmd == 'a') move<Move::LEFT>();
			else if (cmd == 'd') move<Move::RIGHT>();
			else if (('1' <= cmd && cmd <= '9') || cmd == Sudoku::blank()) set(cmd);
			else if (cmd == '=') levelUp();
			else if (cmd == '-') levelDown();
			else if (cmd == 'r') reset();
			else if (cmd == 'n') { mSudoku.generate(); reset(); }
			gotoPos();
			hint();

			// show info
			if (cmd == 'c') check();
			else if (cmd == 'h') hint(!mEnableHint);

			checkSuccess();
		} while (cmd != 'q');
	}

private:
	void help() const {
		paint(mControlPosY, mXMargin, string(static_cast<size_t>(mXBorder - mXMargin), '-'));
		paint(mControlPosY + 1, mXMargin + gXBlankMargin, "move:w/a/s/d   set:1-9.   reset:r   new:n");
		paint(mControlPosY + 2, mXMargin + gXBlankMargin, "check:c   hint:h[OFF]   level:-/=[" + to_string(gLevelMin) + "-" + to_string(gLevelMax) + "]:" + to_string(mLevel));
		paint(mControlPosY + 3, mXMargin + gXBlankMargin, "quit:q");
	}

	template<Move T> void move();
	template<>
	void move<Move::UP>() {
		if (mY > 0) mY -= 1;
	}
	template<>
	void move<Move::DOWN>() {
		if (mY < mPosMap.size() - 1) mY += 1;
	}
	template<>
	void move<Move::LEFT>() {
		if (mX > 0) mX -= 1;
	}
	template<>
	void move<Move::RIGHT>() {
		if (mX < mPosMap.size() - 1) mX += 1;
	}

	void gotoPos() const {
		static int preY = -1;
		static int preX = -1;
		if ((mY != preY || mX != preX) && mUserSudoku) {
			if (preY >= 0 && preX >= 0) clearCross(preY, preX);
			paintCross(mY, mX);
			preY = mY;
			preX = mX;
		}
		const Pos& pos = mPosMap[mY][mX];
		gotoYx(pos.y, pos.x);
	}

	bool set(char val) {
		if (mSudoku[mY][mX] != Sudoku::blank()) return false;
		if (!mSudoku.inRange(val) && val != Sudoku::blank()) return false;

		if (mUserSudoku[mY][mX] == Sudoku::blank() && val != Sudoku::blank()) mBlanks--;
		else if (mUserSudoku[mY][mX] != Sudoku::blank() && val == Sudoku::blank()) mBlanks++;
		mUserSudoku[mY][mX] = val;
		paint(mPosMap[mY][mX], (COLOR_BG_REVERSE + colorPos(mY, mX) + val).c_str());

#ifdef _DEBUG
		gotoYx(30, 0);
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				cout << mUserSudoku[i][j] << " ";
			}
			cout << endl;
		}
		gotoPos();
#endif

		return true;
	}

	string colorPos(const Pos& pos, string defaultColor = COLOR_CLEAR) const {
		return colorPos(pos.y, pos.x, defaultColor);
	}
	string colorPos(int y, int x, string defaultColor = COLOR_CLEAR) const {
		if (mSudoku && mSudoku.value(y, x) == Sudoku::blank()) return string(COLOR_FG_GREEN);
		return defaultColor;
	}

	void clearCross(int y, int x) const {
		paintCross(y, x, COLOR_CLEAR);
	}

	void paintCross(int y, int x, string defaultColor = COLOR_BG_UNDERLINE) const {
		int size = mUserSudoku.size();
		for (int i = 0; i < size; i++) {
			paintNoBack(mPosMap[y][i], (defaultColor + colorPos(y, i, defaultColor) + mUserSudoku.value(y, i)).c_str());
			paintNoBack(mPosMap[i][x], (defaultColor + colorPos(i, x, defaultColor) + mUserSudoku.value(i, x)).c_str());
		}
		int cellSize = mUserSudoku.cellSize();
		int cellY = y / cellSize * cellSize;
		int cellX = x / cellSize * cellSize;
		for (int i = cellY; i < cellY + 3; i++) {
			for (int j = cellX; j < cellX + 3; j++) {
				paintNoBack(mPosMap[i][j], (defaultColor + colorPos(i, j, defaultColor) + mUserSudoku.value(i, j)).c_str());
			}
		}
		paintNoBack(mPosMap[y][x], (string(COLOR_BG_REVERSE) + mUserSudoku.value(y, x)).c_str());
	}

	bool checkSuccess() {
		if (mBlanks != 0) return false;
		if (!mUserSudoku.check()) {
			paintInfo("FAILED");
			return false;
		}

		paintInfo("PASS");
		return true;
	}

	void check() {
		paintInfo("Checking...");
		string status = "Can NOT Solve!";
		if (mUserSudoku.solve()) status = "Can Solve!" + string(4, ' ');
		paintInfo(status);
	}

	void clearInfo() const {
		paint(mInfoPosY, mXMargin + gXBlankMargin, string(mXBorder, ' '));
	}

	template<typename T>
	void paintInfo(T val) {
		mInfoString.clear();
		paintAppendInfo(val);
	}
	template<typename T>
	void paintAppendInfo(T val) {
		clearInfo();
		if (mInfoString.empty()) {
			mInfoString = val;
		}
		else {
			mInfoString += (string(" -- ") + val);
		}
		paint(mInfoPosY, mXMargin + gXBlankMargin, mInfoString);
	}


	template<typename T>
	void paintNoBack(int y, int x, T val) const {
		gotoYx(y, x);
		cout << COLOR_CLEAR << val << COLOR_CLEAR;
	}

	template<typename T>
	void paintNoBack(const Pos& pos, T val) const {
		paintNoBack(pos.y, pos.x, val);
	}

	template<typename T>
	void paint(int y, int x, T val) const {
		gotoYx(y, x);
		cout << COLOR_CLEAR << val << COLOR_CLEAR;
		gotoPos();
	}

	template<typename T>
	void paint(const Pos& pos, T val) const {
		paint(pos.y, pos.x, val);
	}

	void reset() {
		mUserSudoku = mSudoku;
		reset(mUserSudoku.data());
		paintCross(mY, mX);
		gotoPos();
	}

	void reset(sudoku_t& sudoku) {
		int size = static_cast<int>(sudoku.size());
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				paint(mPosMap[y][x], (colorPos(y, x) + sudoku[y][x]).c_str());
			}
		}
	}

	int levelConvert(int level) const {
		if (level <= gLevelMin) return gMinLevel;
		if (level >= gLevelMax) return puzzle::gMaxLevel;
		return level * 9; // 1 * 9 = 9 ... 7 * 9 = 63 --> 64(maxLevel)
	}

	void resetByLevel() {
		paintInfo("Generating...");
		reset(mPlaceholder);

		mSudoku.generate(levelConvert(mLevel));
		mBlanks = 0;
		int size = mSudoku.size();
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				if (mSudoku[y][x] == Sudoku::blank()) mBlanks++;
			}
		}
		reset();

		clearInfo();
		paint(mControlPosY + 2, mXMargin + 41, mLevel);
	}

	void levelUp() {
		mLevel = min(mLevel + 1, gLevelMax);
		resetByLevel();
	}

	void levelDown() {
		mLevel = max(mLevel - 1, gLevelMin);
		resetByLevel();
	}

	void initialize() noexcept {
		int size = mSudoku.size();
		mXBorder = mXMargin + size * (1 + gXBlankMargin * 2);
		mYBorder = mYMargin + size * (1 + gYBlankMargin);
		mInfoPosY = mYMargin + size * (1 + gYBlankMargin) + 1;
		mControlPosY = mYMargin + size * (1 + gYBlankMargin) + 2;

		for (int y = 0; y < size; y++) {
			mPosMap.emplace_back(vector<Pos>(size, { 0, 0 }));
			mPlaceholder.emplace_back(string(size, '?'));
		}
		mPosMap[0][0] = { mYMargin, mXMargin + gXBlankMargin };
		for (int y = 1; y < size; y++) {
			mPosMap[y][0] = { mPosMap[y - 1][0].y + gYBlankMargin + 1, mPosMap[y - 1][0].x };
		}
		for (int x = 1; x < size; x++) {
			mPosMap[0][x] = { mPosMap[0][x - 1].y, mPosMap[0][x - 1].x + gXBlankMargin * 2 + 1 };
		}
		for (int y = 1; y < size; y++) {
			for (int x = 1; x < size; x++) {
				mPosMap[y][x] = { mPosMap[y - 1][x].y + gYBlankMargin + 1, mPosMap[y][x - 1].x + gXBlankMargin * 2 + 1 };
			}
		}
	}

	void hint() {
		hint(mEnableHint);
	}

	void hint(bool enable) {
		if (enable != mEnableHint) {
			paint(mControlPosY + 2, mXMargin + 18, enable ? "[ON] " : "[OFF]");
			if (!enable) clearInfo();
		}
		mEnableHint = enable;

		if (!mEnableHint) return;
		if (mUserSudoku[mY][mX] != Sudoku::blank()) return;

		vector<char> validValue;
		for (char c = '1'; c <= '0' + mUserSudoku.size(); c++) {
			if (mUserSudoku.canSetValue(mY, mX, c)) validValue.emplace_back(c);
		}

		string hintInfo = "hint(" + to_string(mY + 1) + "," + to_string(mX + 1) + "): [ ";
		for (auto c : validValue) hintInfo = hintInfo + c + ' ';
		hintInfo += ']';
		paintInfo(hintInfo);
	}

private:
	int mX = 0;
	int mY = 0;
	int mXMargin = 1;
	int mYMargin = 1;
	int mXBorder = 0;
	int mYBorder = 0;
	int mInfoPosY = 0;
	int mControlPosY = 0;
	int mLevel = 3;
	int mBlanks = 0;
	bool mEnableHint = false;
	string mInfoString = "";

	vector<vector<Pos>> mPosMap;

	Sudoku mSudoku;
	Sudoku mUserSudoku;
	sudoku_t mPlaceholder;
};