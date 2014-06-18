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

#include "t_128.h"

//game mode flags
#define MODE_SOLVING			0	//unused, keep solving
#define MODE_STOP_PROCESSING	1	//solved or error
#define MODE_STOP_GUESSING		2	//necessary solutions found

struct tripletMask {
	t_128 self;
	t_128 adjacentLine;
	t_128 adjacentBox;
};

//struct fsss2;

//struct solutionProcessorPrintUnique {
//	int n;
//	char firstSolution[88];
//	solutionProcessorPrintUnique();
//};
//
//typedef int (*solverCallBack)(void* context, char* result);

struct fsss2 {
private:
	//bits 0..80 have 1 if the digit has candidate in the respective cell, 0 if solved or eliminated
	//bits 81..108 have 1 if the house (row, column, box) is not solved, 0 if solved
	//bits 109..127 are not used and must be 0
	bm128 grid[9];

	//bits 0..80 have 1 if the respective cell is given or solved, 0 if unsolved
	bm128 solved;

	//NULL or pointer to the class that collects solutions
	void* theProcessor;

	//NULL or pointer to the callback routine that notifies the processor class on each solution found
	//solverCallBack solutionHandler;

	//NULL or pointer to buffer for solved cells
	char* sol;

	//0 = continue solving; 1 = contradiction found, backtrack and continue; 3 = all necessary solutions are found, stop
	int mode;						//combination of the game mode flags, initial 0

	////marker not to make second attempt in searching for line-box eliminations
	//int lockedCandidatesDone;

	unsigned long long numSolutionsToDo;

	//bits to clear when solving particular digit and cell, including the houses at bits 81+
	static const t_128 visibleCells[81];

	//1 for bits in the respective house (9 rows, 9 columns, 9 boxes)
	static const t_128 bitsForHouse[27];

	//line-box interactions
	static const tripletMask tripletMasks[54];

	//clear the context
	void initEmpty();

	//called when each valid solution is found. Can manipulate "mode" value to consider whether to continue with next solutions or stop.
	void solutionFound();

	//updates the context by marking the given digit/cell as solved
	void setDigit(int digit, int cell);

	//does optimized context update with initial givens immediately followed by the actual solution process
	void initGivens(const char* const in);

	//resolves cells with a single candidate for a cell
	void doNakedSingles();

	//resolves cells with single candidate for a house
	void doHiddenSingles();

	//resolves cells for specified digit with single candidate for a house then checks for single candidates for a cell
	void doHiddenSinglesForDigit(int d);

	void doHiddenSinglesForDigitCell(int d, int c);

	//performs line-box eliminations
	void doLockedCandidates();

	//performs line-box eliminations for the specified digit
	static void doLockedCandidatesForDigit(bm128& tmp);

	//void doLockedCandidatesForCell(int d, int c);

	//encapsulates the elimination logic up to T&E
	void doDirectEliminations();

	//does the direct eliminations, then does T&E
	void doEliminations();

	//encapsulates the logic for T&E
	void guess();

	//does a try, then restores the context
	void guess1(int digit, int cell);

	//used by T&E for optimal digit/cell selection
	void findBiValueCell(int& digit, int& cell, int& digit2);

	//handler implementation that solves up to the second solution
	//static int uniqueHandler(void* context, char* result);
public:

	//solver's entry point
	//unsigned long long solveUnique(const char* const in, unsigned long long nSolutions, char* out = NULL);
	unsigned long long solve(const char* const in, unsigned long long nSolutions, char* out = NULL);
	bool isIrreducible(const char* const in);
};

#endif /* SOLVER_H_ */
