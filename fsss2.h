/*
 * fsss2.h
 *
 *  Created on: May 13, 2014
 *      Author: Mladen Dobrichev
 */

//Fast Simple Sudoku Solver 2

#ifndef SOLVER_H_
#define SOLVER_H_
#include <immintrin.h>
#include <memory.h>

#include "t_128.h"

//#define COUNT_TRIALS

//for low-clue pencilmark-only sudoku
//#define GUESS_STRATEGY_2

#define USE_LOCKED_CANDIDATES
#define LOCKED_CANDIDATES_ALWAYS
#define LOCKED_CANDIDATES_USE_CACHE

//#define USE_SUBSETS

#ifdef USE_LOCKED_CANDIDATES
struct tripletMask {
	t_128 self;
	t_128 adjacentLine;
	t_128 adjacentBox;
};
#endif

struct pencilmarks {
	bm128 pm[9];
	void clear() {
		for(int i = 0; i < 9; i++) {
			pm[i].clear();
		}
	}
	const bm128& operator[](int digit) const {
		return pm[digit];
	}
	bm128& operator[](int digit) {
		return pm[digit];
	}
	void forceCell(int cell, int digit) { //forbid all digits except one
		for(int d = 0; d < 9; d++) {
			if(d == digit) continue;
			pm[d].setBit(cell);
		}
	}
	pencilmarks& allowSolution(char* sol) {
		for(int c = 0; c < 81; c++) {
			pm[sol[c] - 1].clearBit(c);
		}
		return *this;
	}
	pencilmarks& fromSolver(const bm128* solverPM) {
		for(int d = 0; d < 9; d++) {
			//pm[d] = constraints::mask81;
			pm[d] = (t_128){0xFFFFFFFFFFFFFFFF,    0x0001FFFF};
			pm[d].clearBits(solverPM[d]); //allow all active pencilmarks
		}
		return *this; //solution pencilmarks must be allowed at some point, see allowSolution(char* sol)
	}
};

struct constraints {
	//1 for first 81 bits
	static const t_128 mask81;

	//1 for first 81 bits (cells) and 27 bits from position 96+ (houses)
	static const t_128 mask108;

	//1 for 27 bits at position 96 (houses)
	//static const t_128 mask27;

	//1 for bits to clear when solving particular cell, including the 20 visible cells, self, and the 3 houses at bits 96+
	static const t_128 visibleCells[81];

	//1 for bits in the respective house (9 rows, 9 columns, 9 boxes)
	static const t_128 bitsForHouse[27];

#ifdef USE_LOCKED_CANDIDATES
	static const tripletMask tripletMasks[54];
#endif

	//static const uint32_t topCellsHouses; //1 for the houses having cells only within top 64 ones
};

template < class X > class fsss2 {
public:
	//data model

	//bits
	//0         1         2         3         4         5         6         7         8         9        10        11        12
	//01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567

	//sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss...............................................
	//bits 0..80 have 1 if the respective cell is given or solved, 0 if unsolved
	bm128 solved; //already solved cells

	//ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc...............RRRRRRRRRCCCCCCCCCBBBBBBBB......
	//l = bits 0..80 have 1 if the digit has candidate in the respective cell, 0 if solved or eliminated
	//RCB = bits 96..121 have 1 if the house (row, column, box) is not solved, 0 if solved
	//bits 81..95 and 122..127 are not used and must be 0
	bm128 grid[9]; //candidates for each value from 1 to 9

	bm128 knownNoHiddenSingles[9]; //cache for latest known candidates for each value that failed in search for hidden single

#ifdef USE_LOCKED_CANDIDATES
#ifdef LOCKED_CANDIDATES_USE_CACHE
	bm128 knownNoLockedCandidates[9];
#endif
#endif

	bm128 trialCandidates; //cache for latest optimal cells to make T&E for

	//the backtracking stack
	bm128 contexts[81][11]; //9 for the candidates per digit + 1 for solved cells + 1 for trial candidates

	//0 = continue solving; 1 = contradiction found, backtrack and continue; 3 = all necessary solutions are found, stop
	uint32_t mode; //combination of the game mode flags, initial 0

	uint32_t guessDepth;

#ifdef USE_LOCKED_CANDIDATES
	int lockedDone;
#endif

#ifdef USE_SUBSETS
	int subsetsDone;
	bm128 knownNoSubsets[36];
#endif

	X &collector; //the object instance that receives notifications for solved cells and solutions found

	//clear the context
	constexpr void inline initEmpty();

	//resolves cells with a single candidate for a cell
	void inline doNakedSingles();

#ifdef USE_LOCKED_CANDIDATES
	//performs line-box eliminations for the specified digit
	static bool doLockedCandidatesForDigit(bm128& tmp);
#endif

	//does the direct eliminations, then does T&E
	void inline doEliminations();

	//used by T&E for optimal digit/cell selection
	int inline findLeastPopulatedCells(bm128& bivalues) const;

	fsss2();

//public:
	fsss2(X &theCollector);
	//solver's entry points
	void inline solve(const char* const in); //givens are bytes from 0 to 9
	void inline solve(const uint16_t* const in); //givens are pencilmarks, bits 0..8 = 1 if the respective value is allowed and 0 otherwise, bits 9..15 are unused and must be 0
	void inline solve(const pencilmarks& forbidden); //givens are forbidden pencilmarks
	void setCellValue(int pos, int value);
	void eliminateCellValue(int pos, int value);
};

//implemented operations
class nullCollector {
public:
	inline bool solutionFound(); //false = continue, true = stop solving
	inline void setCellValue(int cell, int val);
	bool beforeGuess(int guessDepth, int &optCell, int &optDigit);
};

//test whether a given puzzle has at least one solution
class hasAnySolution final : public nullCollector {
	//fsss2<hasAnySolution> solver;
	int nsol;
public:
	//hasAnySolution();
	bool solutionFound();
	int solve(const char* p);
	int solve(const pencilmarks& p);
};

//test whether a solution can be reached without guesses
class noGuess final : public nullCollector {
	fsss2<noGuess>* solverPtr;
	int nsol;
public:
	bool beforeGuess(int guessDepth, int &optCell, int &optDigit);
	bool solutionFound();
	int solve(const char* p);
	int solve(const pencilmarks& p);
	int reduce(pencilmarks& p);
};

//test whether a given puzzle has exactly one solution
class hasSingleSolution final : public nullCollector {
	//fsss2<hasSingleSolution> solver;
	int nsol;
public:
	//hasSingleSolution();
	bool solutionFound();
	int solve(const char* p);
	int solve(const pencilmarks& p);
};

//test whether a given subgrid has at least one redundant given, works for single-solution puzzles only
class isRedundant final : public nullCollector {
public:
	int nsol;
	//char sol[81]; //debug
	bool solutionFound();
	bool solve(const char* p, int testPosition);
	bool solve(const pencilmarks& forbiddenValuePositions, int testValue, int testPosition);
	//void setCellValue(int cell, int val); //debug
};

//test whether a given subgrid has at least one redundant given, works for single-solution puzzles only
class isIrreducible final : public nullCollector {
public:
	int nsol;
	bool solutionFound();
	bool solve(const char* p);
};

//test whether a given puzzle has single solution and returns it
class getSingleSolution : public nullCollector {
protected:
	int nsol;
	char *resChar;
public:
	inline void setCellValue(int cell, int val);
	bool solutionFound();
	int solve(const char* p, char* res);
	int solve(const pencilmarks& forbiddenValuePositions, char* res);
};

//test whether a given puzzle has exactly one solution and return it and guess counters
class singleSolutionGuesses final : public nullCollector {
	int nsol;
	char *resChar;
	int* numGuesses;
public:
	inline void setCellValue(int cell, int val);
	bool beforeGuess(int guessDepth, int &optCell, int &optDigit);
	bool solutionFound();
	int solve(const char* p, char* res, int* numGuessesByDepth);
	int solve(const pencilmarks& p, char* res, int* numGuessesByDepth);
};

//test whether a given puzzle has 2+ solutions and returns two of them
class getTwoSolutions final : public getSingleSolution {
public:
	inline void setCellValue(int cell, int val);
	bool solutionFound();
	int solve(const char* p, char* res);
	int solve(const pencilmarks& forbiddenValuePositions, char* res);
};

//test whether a given puzzle has at least one solution and returns it
class getAnySolution final : public getSingleSolution {
public:
	bool solutionFound();
	int solve(const char* p, char* res);
	int solve(const pencilmarks& forbiddenValuePositions, char* res);
};

//compose pencilmarks from all solutions
class multiSolutionPM final : public nullCollector {
	int nsol;
	pencilmarks resPM; //solutionFound sets bits from sol[] here
	char sol[81]; //setCellValue accumulates the solution here
	int solutionsLimit;
public:
	inline void setCellValue(int cell, int val);
	bool solutionFound();
	int solve(const char* p, pencilmarks& res, int maxSolutions = 0);
	int solve(const pencilmarks& forbiddenValuePositions, pencilmarks& res, int maxSolutions); //goes trough all solutions when maxSolutions = 0
	int solve(const pencilmarks& forbiddenValuePositions, pencilmarks& res); //tries cell by cell
};

//get only the positions of givens and find all minimal unique puzzles within the pattern
class patEnum final : public nullCollector {
	bm128 unsetCells[81]; //for each guess
	uint32_t cellCandidates[82]; //for each guess
	uint32_t chosenGuessCell[82]; //for each guess
	uint32_t usedValues[82]; //accumulated set values
	fsss2<patEnum> solver;
	int nsol;
	int size;
	int numFixedValues;
	int curGuessDepth; //0 to ...
	int numPuzzles;
	//char resChar[88];
	char pp[88];
public:
	patEnum();
	bool solutionFound();
	//void setCellValue(int cell, int val);
	int solve(const char* p, const bm128* fixed = NULL);
	//int solve(const bm128* p);
	bool beforeGuess(int guessDepth, int &optCell, int &optDigit);
	void init(const char *puz, const bm128* fixed);
};

#endif /* SOLVER_H_ */
