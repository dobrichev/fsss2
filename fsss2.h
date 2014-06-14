/*
 * fsss2.h
 *
 *  Created on: May 13, 2014
 *      Author: mladen
 */

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

struct fsss2;

struct solutionProcessorPrintUnique {
	int n;
	char firstSolution[88];
	solutionProcessorPrintUnique();
};

typedef int (*solverCallBack)(void* context, char* result);

struct fsss2 {
private:
	bm128 grid[9];
	bm128 solved;
	void* theProcessor;
	solverCallBack solutionHandler;
	char* sol;
	int mode;						//combination of the game mode flags, initial 0
	int lockedCandidatesDone;

	static const t_128 visibleCells[81];
	static const t_128 bitsForHouse[27];
	static const tripletMask tripletMasks[54];

	void initEmpty();
	void solutionFound();
	void setDigit(int zeroDigit, int cell);
	void initGivens(const char* const in);
	void doNakedSingles();
	void doHiddenSingles();
	void doHiddenSinglesForDigit(int d);
	//void doHiddenSinglesForDigitCell(int d, int c);
	void doLockedCandidates();
	static void doLockedCandidatesForDigit(bm128& tmp);
	//void doLockedCandidatesForCell(int d, int c);
	void doDirectEliminations() __restrict;
	void doEliminations() __restrict;
	void guess();
	void guess1(int digit, int cell);
	void findBiValueCell(int& digit, int& cell, int& digit2);
	static int uniqueHandler(void* context, char* result);
public:
	void solveUnique(const char* const __restrict in, char* __restrict out);
};

#endif /* SOLVER_H_ */
