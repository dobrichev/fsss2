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

//#define USE_LOCKED_CANDIDATES
//#define USE_SUBSETS

#ifdef USE_LOCKED_CANDIDATES
struct tripletMask {
	t_128 self;
	t_128 adjacentLine;
	t_128 adjacentBox;
};
#endif

//implemented operations
class nullCollector {
public:
	inline bool solutionFound(); //false = continue, true = stop solving
	inline void setCellValue(int cell, int val);
};

//test whether a given puzzle has at least one solution
class hasAnySolution : public nullCollector {
	int nsol;
public:
	bool solutionFound();
	int solve(const char* p);
};

//test whether a given puzzle has exactly one solution
class hasSingleSolution : public nullCollector {
	int nsol;
public:
	bool solutionFound();
	int solve(const char* p);
};

//test whether a given subgrid has at least one redundant given, works for multiple-solution puzzles too
class isIrreducible : public nullCollector {
public:
	int nsol;
	bool solutionFound();
	bool solve(const char* p);
};

//test whether a given single-solution puzzle has at least one redundant given
class getSingleSolution : public nullCollector {
//public:
//	int solve(const char* p, char *dest);
};

template < class X > class fsss2 {
private:
	//bits 0..80 have 1 if the digit has candidate in the respective cell, 0 if solved or eliminated
	//bits 81..108 have 1 if the house (row, column, box) is not solved, 0 if solved
	//bits 109..127 are not used and must be 0
	bm128 grid[9];

	//bits 0..80 have 1 if the respective cell is given or solved, 0 if unsolved
	bm128 solved;

	bm128 knownNoHiddenSingles[9];

#ifdef USE_LOCKED_CANDIDATES
	bm128 knownNoLockedCandidates[9];
#endif

	//0 = continue solving; 1 = contradiction found, backtrack and continue; 3 = all necessary solutions are found, stop
	int mode;						//combination of the game mode flags, initial 0

	int guessDepth;
	bm128 contexts[81][10];

#ifdef USE_LOCKED_CANDIDATES
	int lockedDone;
#endif

#ifdef USE_SUBSETS
	int subsetsDone;
#endif

	//bits to clear when solving particular cell, including the 20 visible cells, and the 3 houses at bits 96+
	static const t_128 visibleCells[81];

	//1 for bits in the respective house (9 rows, 9 columns, 9 boxes)
	static const t_128 bitsForHouse[27];

#ifdef USE_LOCKED_CANDIDATES
	static const tripletMask tripletMasks[54];
#endif
	static const t_128 mask81;
	static const t_128 mask108;

	//clear the context
	void inline initEmpty();

	//resolves cells with a single candidate for a cell
	void inline doNakedSingles();

#ifdef USE_LOCKED_CANDIDATES
	//performs line-box eliminations for the specified digit
	static void doLockedCandidatesForDigit(bm128& tmp);
#endif

	//does the direct eliminations, then does T&E
	void inline doEliminations();

	//used by T&E for optimal digit/cell selection
	void inline findLeastPopulatedCells(bm128& bivalues) const;

	X &collector;

	fsss2();

public:
	fsss2(X &theCollector);
	//solver's entry points
	void inline solve(const char* const in);
	void inline solve(const uint16_t* const in);
};


#endif /* SOLVER_H_ */
