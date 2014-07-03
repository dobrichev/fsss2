/*
 * fsss2.cpp
 *
 *  Created on: May 13, 2014
 *      Author: Mladen Dobrichev
 */

//Fast Simple Sudoku Solver 2

#include <stdio.h>
#include <memory.h>
#include "fsss2.h"

//game mode flags
#define MODE_SOLVING			0	//unused, keep solving
#define MODE_STOP_PROCESSING	1	//solved or error
#define MODE_STOP_GUESSING		2	//necessary solutions found

const t_128 fsss2::visibleCells[81] = { //1 for all 20 visible cells, 0 for the cell itself, 1 for the three houses
	{0x80402010081C0FFE,0x0000000804020100},
	{0x00804020101C0FFD,0x0000000808020201},
	{0x01008040201C0FFB,0x0000000810020402},
	{0x0201008040E071F7,0x0000001020020804},
	{0x0402010080E071EF,0x0000001040021008},
	{0x0804020100E071DF,0x0000001080022010},
	{0x10080402070381BF,0x0000002100024020},
	{0x201008040703817F,0x0000002200028040},
	{0x40201008070380FF,0x0000002400030080},
	{0x80402010081FFC07,0x0000000804040100},
	{0x00804020101FFA07,0x0000000808040201},
	{0x01008040201FF607,0x0000000810040402},
	{0x0201008040E3EE38,0x0000001020040804},
	{0x0402010080E3DE38,0x0000001040041008},
	{0x0804020100E3BE38,0x0000001080042010},
	{0x1008040207037FC0,0x0000002100044020},
	{0x201008040702FFC0,0x0000002200048040},
	{0x402010080701FFC0,0x0000002400050080},
	{0x804020100FF80E07,0x0000000804080100},
	{0x0080402017F40E07,0x0000000808080201},
	{0x0100804027EC0E07,0x0000000810080402},
	{0x0201008047DC7038,0x0000001020080804},
	{0x0402010087BC7038,0x0000001040081008},
	{0x08040201077C7038,0x0000001080082010},
	{0x1008040206FF81C0,0x0000002100084020},
	{0x2010080405FF81C0,0x0000002200088040},
	{0x4020100803FF81C0,0x0000002400090080},
	{0x8040E07FF0040201,0x0000004004100100},
	{0x0080E07FE8080402,0x0000004008100201},
	{0x0100E07FD8100804,0x0000004010100402},
	{0x0207038FB8201008,0x0000008020100804},
	{0x0407038F78402010,0x0000008040101008},
	{0x0807038EF8804020,0x0000008080102010},
	{0x10381C0DF9008040,0x0000010100104020},
	{0x20381C0BFA010080,0x0000010200108040},
	{0x40381C07FC020100,0x0000010400110080},
	{0x8040FFE038040201,0x0000004004200100},
	{0x0080FFD038080402,0x0000004008200201},
	{0x0100FFB038100804,0x0000004010200402},
	{0x02071F71C0201008,0x0000008020200804},
	{0x04071EF1C0402010,0x0000008040201008},
	{0x08071DF1C0804020,0x0000008080202010},
	{0x10381BFE01008040,0x0000010100204020},
	{0x203817FE02010080,0x0000010200208040},
	{0x40380FFE04020100,0x0000010400210080},
	{0x807FC07038040201,0x0000004004400100},
	{0x00BFA07038080402,0x0000004008400201},
	{0x013F607038100804,0x0000004010400402},
	{0x023EE381C0201008,0x0000008020400804},
	{0x043DE381C0402010,0x0000008040401008},
	{0x083BE381C0804020,0x0000008080402010},
	{0x1037FC0E01008040,0x0000010100404020},
	{0x202FFC0E02010080,0x0000010200408040},
	{0x401FFC0E04020100,0x0000010400410080},
	{0xFF80201008040201,0x0000020004800703},
	{0xFF40402010080402,0x0000020008800703},
	{0xFEC0804020100804,0x0000020010800703},
	{0x7DC1008040201008,0x000004002080381C},
	{0x7BC2010080402010,0x000004004080381C},
	{0x77C4020100804020,0x000004008080381C},
	{0x6FC8040201008040,0x000008010081C0E0},
	{0x5FD0080402010080,0x000008020081C0E0},
	{0x3FE0100804020100,0x000008040081C0E0},
	{0x01C0201008040201,0x00000200050007FF},
	{0x81C0402010080402,0x00000200090007FE},
	{0x81C0804020100804,0x00000200110007FD},
	{0x8E01008040201008,0x00000400210038FB},
	{0x8E02010080402010,0x00000400410038F7},
	{0x8E04020100804020,0x00000400810038EF},
	{0xF008040201008040,0x000008010101C0DF},
	{0xF010080402010080,0x000008020101C0BF},
	{0xF020100804020100,0x000008040101C07F},
	{0x81C0201008040201,0x000002000601FE03},
	{0x81C0402010080402,0x000002000A01FD03},
	{0x81C0804020100804,0x000002001201FB03},
	{0x0E01008040201008,0x000004002201F71C},
	{0x0E02010080402010,0x000004004201EF1C},
	{0x0E04020100804020,0x000004008201DF1C},
	{0x7008040201008040,0x000008010201BFE0},
	{0x7010080402010080,0x0000080202017FE0},
	{0x7020100804020100,0x000008040200FFE0},
}; //visibleCells[81]

const t_128 fsss2::bitsForHouse[27] = { //1 for the 9 cells in the house
	{0x00000000000001FF,0x0000000000000000},
	{0x000000000003FE00,0x0000000000000000},
	{0x0000000007FC0000,0x0000000000000000},
	{0x0000000FF8000000,0x0000000000000000},
	{0x00001FF000000000,0x0000000000000000},
	{0x003FE00000000000,0x0000000000000000},
	{0x7FC0000000000000,0x0000000000000000},
	{0x8000000000000000,0x00000000000000FF},
	{0x0000000000000000,0x000000000001FF00},
	{0x8040201008040201,0x0000000000000100},
	{0x0080402010080402,0x0000000000000201},
	{0x0100804020100804,0x0000000000000402},
	{0x0201008040201008,0x0000000000000804},
	{0x0402010080402010,0x0000000000001008},
	{0x0804020100804020,0x0000000000002010},
	{0x1008040201008040,0x0000000000004020},
	{0x2010080402010080,0x0000000000008040},
	{0x4020100804020100,0x0000000000010080},
	{0x00000000001C0E07,0x0000000000000000},
	{0x0000000000E07038,0x0000000000000000},
	{0x00000000070381C0,0x0000000000000000},
	{0x0000E07038000000,0x0000000000000000},
	{0x00070381C0000000,0x0000000000000000},
	{0x00381C0E00000000,0x0000000000000000},
	{0x81C0000000000000,0x0000000000000703},
	{0x0E00000000000000,0x000000000000381C},
	{0x7000000000000000,0x000000000001C0E0},
}; //bitsForHouse[27]

bool fsss2::isIrreducible(const char* const in) {
	int pos[81], val[81], nGivens = 0;
	int dc[9] = {0,0,0,0,0,0,0,0,0};
	for(int c = 0; c < 81; c++) {
		if(in[c] == 0)
			continue;
		pos[nGivens] = c;
		val[nGivens++] = in[c] - 1;
		if(++dc[in[c] - 1] > 6)
			return false; //this works for 36+ givens
	}
	sol = NULL;
	for(int skip = 0; skip < nGivens; skip++) {
	//for(int skip = nGivens - 1; skip >= 0; skip--) { //slower
		initEmpty();
		numSolutionsToDo = 1;
		//set the givens except for the tested cell
		for(int n = 0; n < nGivens; n++) {
		//for(int n = nGivens - 1; n >= 0; n--) { //slower
			if(n != skip) {
				if(!grid[val[n]].isBitSet(pos[n])) {
					//direct contradiction within the initial givens
					mode = MODE_STOP_PROCESSING;
					return false;
				}
				solved.setBit(pos[n]); //mark cell as "solved"
				grid[val[n]].clearBits(visibleCells[pos[n]]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
			}
		}
		//forbid the given for the tested cell
		grid[val[skip]].clearBit(pos[skip]);
		//update the candidates
		grid[0].clearBits(solved);
		grid[1].clearBits(solved);
		grid[2].clearBits(solved);
		grid[3].clearBits(solved);
		grid[4].clearBits(solved);
		grid[5].clearBits(solved);
		grid[6].clearBits(solved);
		grid[7].clearBits(solved);
		grid[8].clearBits(solved);
		//check whether there is at least one solution with the different value for the tested cell
		doEliminations();
		if(numSolutionsToDo) {
			//no solution with different value for the tested cell exists, therefore the given at pos[skip] is redundant
			return false;
		}
	}
	//all tests passed
	return true;
}

unsigned long long fsss2::solve(const char* const in, const unsigned long long nSolutions, char* const out) {
	//start from clean solver context
	initEmpty();
	sol = out;
	numSolutionsToDo = nSolutions;
	//perform optimized setup with the initial givens
	for(int c = 0; c < 81; c++) {
		int d = in[c];
		if(d == 0) {
			//skip non-givens
			continue;
		}
		if(!grid[--d].isBitSet(c)) {
			//direct contradiction within the initial givens
			mode = MODE_STOP_PROCESSING;
			return 0;
		}
		if(sol) {
			//if buffer for the solution is given, store the digit
			sol[c] = d + 1;
		}
		solved.setBit(c); //mark cell as "solved"
		grid[d].clearBits(visibleCells[c]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
	}
	//clear all givens from the candidates in one pass
	grid[0].clearBits(solved);
	grid[1].clearBits(solved);
	grid[2].clearBits(solved);
	grid[3].clearBits(solved);
	grid[4].clearBits(solved);
	grid[5].clearBits(solved);
	grid[6].clearBits(solved);
	grid[7].clearBits(solved);
	grid[8].clearBits(solved);
	if(((bm128)mask81).isSubsetOf(solved)) {
		//all givens :)
		solutionFound();
		return 1;
	}
	//now do the entire solving process
	doEliminations();
	return nSolutions - numSolutionsToDo;
}

void fsss2::solutionFound() {
	if(--numSolutionsToDo) {
		if(sol) {
			memcpy(sol + 81, sol, 81);
			sol += 81;
		}
		mode = MODE_STOP_PROCESSING;
		return;
	}
	mode = MODE_STOP_PROCESSING | MODE_STOP_GUESSING;
}

void fsss2::initEmpty() {
	//set all cells and houses as "unsolved"
	grid[0] = mask108;
	grid[1] = mask108;
	grid[2] = mask108;
	grid[3] = mask108;
	grid[4] = mask108;
	grid[5] = mask108;
	grid[6] = mask108;
	grid[7] = mask108;
	grid[8] = mask108;
	//no solved cells yet
	solved.clear();
	//no processed digits for hidden singles yet
	knownNoHiddenSingles[0].clear();
	knownNoHiddenSingles[1].clear();
	knownNoHiddenSingles[2].clear();
	knownNoHiddenSingles[3].clear();
	knownNoHiddenSingles[4].clear();
	knownNoHiddenSingles[5].clear();
	knownNoHiddenSingles[6].clear();
	knownNoHiddenSingles[7].clear();
	knownNoHiddenSingles[8].clear();
	//should solve
	mode = 0;
}

void fsss2::setDigit(const int d, const int c) {
	if(sol)
		sol[c] = d + 1; //store the digit if solution buffer is given
	solved.setBit(c); //mark cell as "solved"
	if(((bm128)mask81).isSubsetOf(solved)) {
		solutionFound();
		return;
	}
	//clear all digit candidates for this cell
	grid[0].clearBit(c);
	grid[1].clearBit(c);
	grid[2].clearBit(c);
	grid[3].clearBit(c);
	grid[4].clearBit(c);
	grid[5].clearBit(c);
	grid[6].clearBit(c);
	grid[7].clearBit(c);
	grid[8].clearBit(c);
	grid[d].clearBits(visibleCells[c]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
}

inline void fsss2::doNakedSingles() { //cells with only one remaining candidate
again:
	bm128 all = solved;
	bm128 duplicates = solved; //cells with 2 or more candidates
	for(int d = 0; d < 9; d++) {
		bm128 tmp = grid[d];
		tmp &= all;
		duplicates |= tmp;
		all |= grid[d];
	}
	if(!((bm128)mask81).isSubsetOf(all)) {
		//there is at least one unsolved cell without any candidate
		mode = MODE_STOP_PROCESSING;
		return;
	}
	if(((bm128)mask81).isSubsetOf(duplicates)) {
		//sorry, no naked singles
		return;
	}
	all = mask81;
	all.clearBits(duplicates);
	//now find which unique where came from
	unsigned char pos[88];
	int n;
	n = all.getPositions96(pos);
	for(int c = 0; c < n; c++) {
		const bm128& theBit = bitSet[pos[c]];
		const bm128& theCells = visibleCells[pos[c]];
		for(int d = 0; d < 9; d++) {
			if(!theBit.isSubsetOf(grid[d]))
				continue;
			grid[d].clearBits(theCells);
			if(sol)
				sol[pos[c]] = d + 1;
			goto next_pos;
		}
		//this cell has been just cleared by setting other naked single (2 naked in a house for the same digit)
		//now this cell has no candidates which is a contradiction
		mode = MODE_STOP_PROCESSING;
		return;
next_pos:
		;
	}
	//remove all candidates for the solved cells
	grid[0].clearBits(all);
	grid[1].clearBits(all);
	grid[2].clearBits(all);
	grid[3].clearBits(all);
	grid[4].clearBits(all);
	grid[5].clearBits(all);
	grid[6].clearBits(all);
	grid[7].clearBits(all);
	grid[8].clearBits(all);
	solved |= all; //mark cells as solved
	if(((bm128)mask81).isSubsetOf(solved)) {
		//finally all 81 cells are solved
		solutionFound();
		return;
	}
	goto again;
}

void fsss2::doHiddenSingles() { //digits with only one occurrence in a house
	restart:
	int found = 0;
#pragma forceinline recursive
	for(int d = 0; d < 9; d++) { //for each digit
		if(knownNoHiddenSingles[d] == grid[d]) {
			continue;
		}
		again:
		//get only the unsolved houses and iterate them
		//uint32_t houses = ((1 << 27) - 1) & ((grid[d].toInt64_1()) >> (81 - 64)); //if the bits after 81+27 are used and not 0
		uint32_t houses = (grid[d].toInt64_1()) >> (81 - 64); //get bits 64..127 and shift
		while(houses) {
			uint32_t hbm = houses & -houses; //first unsolved house
			houses ^= hbm; //clear this house marking it as processed
			unsigned int h = __builtin_ctz(hbm); //house index
			bm128 tmp = grid[d];
			tmp &= bitsForHouse[h]; //mask other candidates and leave only these from the current house
			//find whether the house has a single candidate and obtain its position
			int cell;
			//exploit the fact that when (x & (x-1)) == 0 then x has 0 or 1 bit set
			const t_128 one = {1,1};
			if(0 == _mm_testz_si128(tmp.bitmap128.m128i_m128i, _mm_sub_epi64(tmp.bitmap128.m128i_m128i, one.m128i_m128i)))
				continue; //too many candidates

			unsigned long long low64; //low 64-bits
			uint32_t high17; //high 81-64=17 bits
			low64 = _mm_cvtsi128_si64(tmp.bitmap128.m128i_m128i);
			high17 = _mm_cvtsi128_si32(_mm_srli_si128(tmp.bitmap128.m128i_m128i, 8));
			if(low64) {
				if(high17) continue; //candidates in both low and high part of the house
				//get the position of the single candidate in the low part of the house
				cell = __builtin_ctzll(low64);
				goto single_found;
			}
			if(high17) {
				//get the position of the single candidate in the high part of the house
				cell = 64 + __builtin_ctz(high17);
				goto single_found;
			}
			//no any candidate for this digit in this unsolved house
			mode = MODE_STOP_PROCESSING;
			return;
single_found:
			;
			setDigit(d, cell); //store the digit and clear the houses
			if(mode) return;
			doNakedSingles(); //checking a single cell, possible eliminations in other digits
			if(mode) return;
			found = 1;
			//checking again the same digit leads to faster contradiction detection
			goto again;
		}
		knownNoHiddenSingles[d] = grid[d];
	}
	if(found) goto restart;
}

void fsss2::doDirectEliminations() {
again:
	doNakedSingles();
	if(mode) return;
	doHiddenSingles();
}

void fsss2::doEliminations() {
	doDirectEliminations();
	if(mode) return;
	guess(); //this calls recursively doEliminations
}

void fsss2::findBiValueCell(int& digit, int& cell, int& digit2) { //cells with 2 remaining candidates
	bm128 all;
	all = solved;
	bm128 duplicates = solved;
	bm128 triplicates = solved;
	for(int d = 0; d < 9; d++) {
		bm128 tmp = grid[d];
		tmp &= all;
		bm128 tmp2 = tmp;
		tmp2 &= duplicates;
		triplicates |= tmp2;
		duplicates |= tmp;
		all |= grid[d];
	}
	all &= mask81; //clear other bits
	all.clearBits(triplicates);
	if(all.isZero()) {
		digit = -1;
		return;
	}
	for(int d = 0; d < 8; d++) {
		if(!all.isDisjoint(grid[d])) {
			all &= grid[d];
			cell = all.getFirstBit1Index96();
			digit = d;
			for(int d2 = d + 1; d2 < 9; d2++) {
				if(grid[d2].isBitSet(cell)) {
					digit2 = d2;
					return;
				}
			}
		}
	}
//	//debug
//	char p[88];
//	p[81] = 0;
//	for(int i = 0; i < 9; i++) {
//		grid[i].toMask81(p);
//		printf("%s\n", p);
//	}
//	solved.toMask81(p);
//	printf("\n%s\n", p);
//	biValues.toMask81(p);
//	printf("\n%s\n", p);
//	printf("*********\n");
}

void fsss2::guess() {
	//Prepare a guess
again:
	//Find an unsolved cell with less possibilities
	int optDigit;
	int optDigit2;
	int optCell;
	int minCells = 100;
	int n;

	//find first bi-value cell and return the two values
	findBiValueCell(optDigit, optCell, optDigit2);
	if(optDigit != -1) {
		//try with the first value
		guess1(optDigit, optCell);
		if(mode) return;
		//we have in hands a secondary digit to set which also automatically clears from the candidates the just examined value
		setDigit(optDigit2, optCell);
		if(mode) return;
	}
	else {
		//find house with less candidates from a particular digit, exit on first bi-position house/digit
		for(int d = 0; d < 9; d++) {
			for(int h = 0; h < 27; h++) {
				if(!grid[d].isBitSet(81 + h))
					continue;
				bm128 tmp = grid[d];
				tmp &= bitsForHouse[h];
				if(tmp.isZero()) {
					continue;
				}
				tmp &= mask81;
				n = tmp.popcount_128();
				if(n < minCells) {
					optDigit = d;
					optCell = tmp.getFirstBit1Index96();
					if(optCell < 0) {
						return;
					}
					if(n == 2)
						break; //not so bad, a bi-position is found
					minCells = n;
				}
			}
		}
		guess1(optDigit, optCell);
		if(mode) return;
		//clear the candidate from further processing
		grid[optDigit].clearBit(optCell);
	}
	//continue after this elimination
	doDirectEliminations(); //don't recurse with doEliminations but loop here with doDirectEliminations
	if(mode) return; //contradiction or sufficient solutions found
	goto again;
}

void fsss2::guess1(int digit, int cell) {
	bm128 gg[10];
	gg[0] = grid[0];
	gg[1] = grid[1];
	gg[2] = grid[2];
	gg[3] = grid[3];
	gg[4] = grid[4];
	gg[5] = grid[5];
	gg[6] = grid[6];
	gg[7] = grid[7];
	gg[8] = grid[8];
	gg[9] = solved;

	setDigit(digit, cell);
	if(mode) return; //on contradiction return w/o consequences
	doEliminations();
	if(mode & MODE_STOP_GUESSING) {
		return;
	}
	//We are done with the guess.
	//The caller is notified for each of the the possible solutions found so far
	//Now restore the context and later in the caller remove this candidate from the further solutions or set the only remaining candidate
	grid[0] = gg[0];
	grid[1] = gg[1];
	grid[2] = gg[2];
	grid[3] = gg[3];
	grid[4] = gg[4];
	grid[5] = gg[5];
	grid[6] = gg[6];
	grid[7] = gg[7];
	grid[8] = gg[8];
	solved = gg[9];
	mode = 0;
}

