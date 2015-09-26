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

//#define USE_LOCKED_CANDIDATES
//#define USE_SUBSETS

#ifdef USE_LOCKED_CANDIDATES
struct tripletMask {
	t_128 self;
	t_128 adjacentLine;
	t_128 adjacentBox;
};
#endif

struct constraints {
	static const t_128 mask81;
	static const t_128 mask108;
	static const t_128 mask27;

	//bits to clear when solving particular cell, including the 20 visible cells, and the 3 houses at bits 96+
	static const t_128 visibleCells[81];

	//1 for bits in the respective house (9 rows, 9 columns, 9 boxes)
	static const t_128 bitsForHouse[27];

#ifdef USE_LOCKED_CANDIDATES
	static const tripletMask tripletMasks[54];
#endif
	//static const uint32_t topCellsHouses;
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
	//bm128 knownNoLockedCandidates[9];
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
	//bm128 knownNoSubsets[36];
#endif

	X &collector; //the object instance that receives notifications for solved cells and solutions found

	//clear the context
	void inline initEmpty();

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
	void setCellValue(int pos, int value);
	void eliminateCellValue(int pos, int value);
};

//implemented operations
class nullCollector {
public:
	inline bool solutionFound(); //false = continue, true = stop solving
	inline void setCellValue(int cell, int val);
};

//test whether a given puzzle has at least one solution
class hasAnySolution : public nullCollector {
	//fsss2<hasAnySolution> solver;
	int nsol;
public:
	//hasAnySolution();
	bool solutionFound();
	int solve(const char* p);
	int solve(const bm128* p);
};

//test whether a given puzzle has exactly one solution
class hasSingleSolution : public nullCollector {
	//fsss2<hasSingleSolution> solver;
	int nsol;
public:
	//hasSingleSolution();
	bool solutionFound();
	int solve(const char* p);
	//int solve(const bm128* p);
};

//test whether a given subgrid has at least one redundant given, works for multiple-solution puzzles too
class isRedundant : public nullCollector {
public:
	int nsol;
	//char sol[81]; //debug
	bool solutionFound();
	bool solve(const char* p, int c);
	bool solve(const bm128* p, int given, int position);
	//void setCellValue(int cell, int val); //debug
};

//test whether a given subgrid has at least one redundant given, works for multiple-solution puzzles too
class isIrreducible : public nullCollector {
public:
	int nsol;
	bool solutionFound();
	bool solve(const char* p);
};

//test whether a given puzzle has single solution and returns it
class getSingleSolution : public nullCollector {
	int nsol;
	char *resChar;
public:
	inline void setCellValue(int cell, int val);
	bool solutionFound();
	int solve(const char* p, char* res);
	int solve(const bm128* p, char* res);
};

#endif /* SOLVER_H_ */
