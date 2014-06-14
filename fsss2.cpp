/*
 * fsss2.cpp
 *
 *  Created on: May 13, 2014
 *      Author: mladen
 */

#include <stdio.h>
#include <memory.h>
#include "fsss2.h"

solutionProcessorPrintUnique::solutionProcessorPrintUnique() : n(0) {};

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

const tripletMask fsss2::tripletMasks[54] = {
	{{0x0000000000000007,0x0000000000000000}, {0x00000000000001F8,0x0000000000000000}, {0x00000000001C0E00,0x0000000000000000}, },
	{{0x0000000000000038,0x0000000000000000}, {0x00000000000001C7,0x0000000000000000}, {0x0000000000E07000,0x0000000000000000}, },
	{{0x00000000000001C0,0x0000000000000000}, {0x000000000000003F,0x0000000000000000}, {0x0000000007038000,0x0000000000000000}, },
	{{0x0000000000000E00,0x0000000000000000}, {0x000000000003F000,0x0000000000000000}, {0x00000000001C0007,0x0000000000000000}, },
	{{0x0000000000007000,0x0000000000000000}, {0x0000000000038E00,0x0000000000000000}, {0x0000000000E00038,0x0000000000000000}, },
	{{0x0000000000038000,0x0000000000000000}, {0x0000000000007E00,0x0000000000000000}, {0x00000000070001C0,0x0000000000000000}, },
	{{0x00000000001C0000,0x0000000000000000}, {0x0000000007E00000,0x0000000000000000}, {0x0000000000000E07,0x0000000000000000}, },
	{{0x0000000000E00000,0x0000000000000000}, {0x00000000071C0000,0x0000000000000000}, {0x0000000000007038,0x0000000000000000}, },
	{{0x0000000007000000,0x0000000000000000}, {0x0000000000FC0000,0x0000000000000000}, {0x00000000000381C0,0x0000000000000000}, },
	{{0x0000000038000000,0x0000000000000000}, {0x0000000FC0000000,0x0000000000000000}, {0x0000E07000000000,0x0000000000000000}, },
	{{0x00000001C0000000,0x0000000000000000}, {0x0000000E38000000,0x0000000000000000}, {0x0007038000000000,0x0000000000000000}, },
	{{0x0000000E00000000,0x0000000000000000}, {0x00000001F8000000,0x0000000000000000}, {0x00381C0000000000,0x0000000000000000}, },
	{{0x0000007000000000,0x0000000000000000}, {0x00001F8000000000,0x0000000000000000}, {0x0000E00038000000,0x0000000000000000}, },
	{{0x0000038000000000,0x0000000000000000}, {0x00001C7000000000,0x0000000000000000}, {0x00070001C0000000,0x0000000000000000}, },
	{{0x00001C0000000000,0x0000000000000000}, {0x000003F000000000,0x0000000000000000}, {0x0038000E00000000,0x0000000000000000}, },
	{{0x0000E00000000000,0x0000000000000000}, {0x003F000000000000,0x0000000000000000}, {0x0000007038000000,0x0000000000000000}, },
	{{0x0007000000000000,0x0000000000000000}, {0x0038E00000000000,0x0000000000000000}, {0x00000381C0000000,0x0000000000000000}, },
	{{0x0038000000000000,0x0000000000000000}, {0x0007E00000000000,0x0000000000000000}, {0x00001C0E00000000,0x0000000000000000}, },
	{{0x01C0000000000000,0x0000000000000000}, {0x7E00000000000000,0x0000000000000000}, {0x8000000000000000,0x0000000000000703}, },
	{{0x0E00000000000000,0x0000000000000000}, {0x71C0000000000000,0x0000000000000000}, {0x0000000000000000,0x000000000000381C}, },
	{{0x7000000000000000,0x0000000000000000}, {0x0FC0000000000000,0x0000000000000000}, {0x0000000000000000,0x000000000001C0E0}, },
	{{0x8000000000000000,0x0000000000000003}, {0x0000000000000000,0x00000000000000FC}, {0x01C0000000000000,0x0000000000000700}, },
	{{0x0000000000000000,0x000000000000001C}, {0x8000000000000000,0x00000000000000E3}, {0x0E00000000000000,0x0000000000003800}, },
	{{0x0000000000000000,0x00000000000000E0}, {0x8000000000000000,0x000000000000001F}, {0x7000000000000000,0x000000000001C000}, },
	{{0x0000000000000000,0x0000000000000700}, {0x0000000000000000,0x000000000001F800}, {0x81C0000000000000,0x0000000000000003}, },
	{{0x0000000000000000,0x0000000000003800}, {0x0000000000000000,0x000000000001C700}, {0x0E00000000000000,0x000000000000001C}, },
	{{0x0000000000000000,0x000000000001C000}, {0x0000000000000000,0x0000000000003F00}, {0x7000000000000000,0x00000000000000E0}, },
	{{0x0000000000040201,0x0000000000000000}, {0x8040201008000000,0x0000000000000100}, {0x0000000000180C06,0x0000000000000000}, },
	{{0x0000201008000000,0x0000000000000000}, {0x8040000000040201,0x0000000000000100}, {0x0000C06030000000,0x0000000000000000}, },
	{{0x8040000000000000,0x0000000000000100}, {0x0000201008040201,0x0000000000000000}, {0x0180000000000000,0x0000000000000603}, },
	{{0x0000000000080402,0x0000000000000000}, {0x0080402010000000,0x0000000000000201}, {0x0000000000140A05,0x0000000000000000}, },
	{{0x0000402010000000,0x0000000000000000}, {0x0080000000080402,0x0000000000000201}, {0x0000A05028000000,0x0000000000000000}, },
	{{0x0080000000000000,0x0000000000000201}, {0x0000402010080402,0x0000000000000000}, {0x8140000000000000,0x0000000000000502}, },
	{{0x0000000000100804,0x0000000000000000}, {0x0100804020000000,0x0000000000000402}, {0x00000000000C0603,0x0000000000000000}, },
	{{0x0000804020000000,0x0000000000000000}, {0x0100000000100804,0x0000000000000402}, {0x0000603018000000,0x0000000000000000}, },
	{{0x0100000000000000,0x0000000000000402}, {0x0000804020100804,0x0000000000000000}, {0x80C0000000000000,0x0000000000000301}, },
	{{0x0000000000201008,0x0000000000000000}, {0x0201008040000000,0x0000000000000804}, {0x0000000000C06030,0x0000000000000000}, },
	{{0x0001008040000000,0x0000000000000000}, {0x0200000000201008,0x0000000000000804}, {0x0006030180000000,0x0000000000000000}, },
	{{0x0200000000000000,0x0000000000000804}, {0x0001008040201008,0x0000000000000000}, {0x0C00000000000000,0x0000000000003018}, },
	{{0x0000000000402010,0x0000000000000000}, {0x0402010080000000,0x0000000000001008}, {0x0000000000A05028,0x0000000000000000}, },
	{{0x0002010080000000,0x0000000000000000}, {0x0400000000402010,0x0000000000001008}, {0x0005028140000000,0x0000000000000000}, },
	{{0x0400000000000000,0x0000000000001008}, {0x0002010080402010,0x0000000000000000}, {0x0A00000000000000,0x0000000000002814}, },
	{{0x0000000000804020,0x0000000000000000}, {0x0804020100000000,0x0000000000002010}, {0x0000000000603018,0x0000000000000000}, },
	{{0x0004020100000000,0x0000000000000000}, {0x0800000000804020,0x0000000000002010}, {0x00030180C0000000,0x0000000000000000}, },
	{{0x0800000000000000,0x0000000000002010}, {0x0004020100804020,0x0000000000000000}, {0x0600000000000000,0x000000000000180C}, },
	{{0x0000000001008040,0x0000000000000000}, {0x1008040200000000,0x0000000000004020}, {0x0000000006030180,0x0000000000000000}, },
	{{0x0008040200000000,0x0000000000000000}, {0x1000000001008040,0x0000000000004020}, {0x0030180C00000000,0x0000000000000000}, },
	{{0x1000000000000000,0x0000000000004020}, {0x0008040201008040,0x0000000000000000}, {0x6000000000000000,0x00000000000180C0}, },
	{{0x0000000002010080,0x0000000000000000}, {0x2010080400000000,0x0000000000008040}, {0x0000000005028140,0x0000000000000000}, },
	{{0x0010080400000000,0x0000000000000000}, {0x2000000002010080,0x0000000000008040}, {0x0028140A00000000,0x0000000000000000}, },
	{{0x2000000000000000,0x0000000000008040}, {0x0010080402010080,0x0000000000000000}, {0x5000000000000000,0x00000000000140A0}, },
	{{0x0000000004020100,0x0000000000000000}, {0x4020100800000000,0x0000000000010080}, {0x00000000030180C0,0x0000000000000000}, },
	{{0x0020100800000000,0x0000000000000000}, {0x4000000004020100,0x0000000000010080}, {0x00180C0600000000,0x0000000000000000}, },
	{{0x4000000000000000,0x0000000000010080}, {0x0020100804020100,0x0000000000000000}, {0x3000000000000000,0x000000000000C060}, },
}; //tripletMasks

int fsss2::uniqueHandler(void* context, char* result) {
	solutionProcessorPrintUnique* sp = (solutionProcessorPrintUnique*)context;
	sp->n++;
	if(sp->n == 2)
		return 1;
	//memcpy(sp->firstSolution, result, 81);
	//for(int i = 0; i < 81; i++) {
	//	sp->firstSolution[i] = result[i] + '1';
	//}
	return 0;
}

void fsss2::solveUnique(const char* const __restrict in, char* __restrict out) {
	char s[81];
	solutionProcessorPrintUnique sp;
	initEmpty();
	theProcessor = &sp;
	sol = s;
	solutionHandler = uniqueHandler;
	initGivens(in);
	if(sp.n == 1) {
		//printf("Unique\n");
		//printf("%81.81s\n", sp.firstSolution);
	}
	else if(sp.n == 0) {
		printf("Invalid\n");
	}
	else {
		printf("Multiple\n");
	}
}

void fsss2::solutionFound() {
	if(solutionHandler) {
		if((*solutionHandler)(theProcessor, sol)) {
			//forced end
			mode = MODE_STOP_PROCESSING | MODE_STOP_GUESSING;
			return;
		}
	}
	mode = MODE_STOP_PROCESSING;
}

void fsss2::initEmpty() {
	grid[0] = maskLSB[81 + 27];
	grid[1] = maskLSB[81 + 27];
	grid[2] = maskLSB[81 + 27];
	grid[3] = maskLSB[81 + 27];
	grid[4] = maskLSB[81 + 27];
	grid[5] = maskLSB[81 + 27];
	grid[6] = maskLSB[81 + 27];
	grid[7] = maskLSB[81 + 27];
	grid[8] = maskLSB[81 + 27];
	solved.clear();
	mode = 0;
	theProcessor = NULL;
	sol = NULL;
}

void fsss2::initGivens(const char* const in) {
	for(int c = 0; c < 81; c++) {
		int d = in[c];
		if(d == 0) {
			continue;
		}
		if(!grid[--d].isBitSet(c)) {
			mode = MODE_STOP_PROCESSING;
			return;
		}
		if(sol)
			sol[c] = d;
		solved.setBit(c); //mark cell as "solved"
		grid[d].clearBits(visibleCells[c]);
	}
	grid[0].clearBits(solved);
	grid[1].clearBits(solved);
	grid[2].clearBits(solved);
	grid[3].clearBits(solved);
	grid[4].clearBits(solved);
	grid[5].clearBits(solved);
	grid[6].clearBits(solved);
	grid[7].clearBits(solved);
	grid[8].clearBits(solved);
	if(((bm128)maskLSB[81]).isSubsetOf(solved)) {
		//all givens :)
		solutionFound();
		return;
	}
	doEliminations();
}

void fsss2::setDigit(int d, int c) {
//	if(!grid[d].isBitSet(c)) {
//		mode = MODE_STOP_PROCESSING;
//		printf(".");
//		return;
//	}

//	//debug
//	bm128 tmp = grid[d];
//	tmp.clearBits(maskLSB[81]);
//	tmp &= visibleCells[c];
//	if(tmp.popcount_128() != 3) {
//		mode = MODE_STOP_PROCESSING;
//		printf(".");
//		return;
//	}

	if(sol)
		sol[c] = d;
	solved.setBit(c); //mark cell as "solved"
	grid[0].clearBit(c);
	grid[1].clearBit(c);
	grid[2].clearBit(c);
	grid[3].clearBit(c);
	grid[4].clearBit(c);
	grid[5].clearBit(c);
	grid[6].clearBit(c);
	grid[7].clearBit(c);
	grid[8].clearBit(c);
	grid[d].clearBits(visibleCells[c]);
	if(((bm128)maskLSB[81]).isSubsetOf(solved)) {
		solutionFound();
		//return;
	}
	//doHiddenSinglesForDigitCell(d, c);
	//if(mode) return;
}

//void fsss2::findNakedSingles(bm128& all) { //cells with only one remaining candidate
//	bm128 duplicates = solved;
//	for(int d = 0; d < 9; d++) {
//		bm128 tmp = grid[d];
//		tmp &= all;
//		duplicates |= tmp;
//		all |= grid[d];
//	}
//	if(((bm128)maskLSB[81]).isSubsetOf(all)) {
//		mode = MODE_STOP_PROCESSING;
//		return;
//	}
//	all &= maskLSB[81]; //clear other bits
////	if(all != maskLSB[81]) { //unsolved cells w/o any candidate
////		mode = MODE_STOP_PROCESSING;
////		return;
////	}
//	all.clearBits(duplicates);
//}

void fsss2::doNakedSingles() { //cells with only one remaining candidate
	do {
		bm128 all = solved;
		bm128 duplicates = solved;
		for(int d = 0; d < 9; d++) {
			bm128 tmp = grid[d];
			tmp &= all;
			duplicates |= tmp;
			all |= grid[d];
		}
		if(!((bm128)maskLSB[81]).isSubsetOf(all)) {
			//there is no any candidate for some unsolved cells
			mode = MODE_STOP_PROCESSING;
			return;
		}
		if(((bm128)maskLSB[81]).isSubsetOf(duplicates)) {
			//sorry, no naked singles
			return;
		}
		all = maskLSB[81];
		all.clearBits(duplicates);
		//now find which unique where came from
		unsigned char pos[88];
		int n;
		n = all.getPositions96(pos);
		for(int c = 0; c < n; c++) {
			for(int d = 0; d < 9; d++) {
				if(grid[d].isBitSet(pos[c])) {
					if(sol)
						sol[pos[c]] = d;
					grid[d].clearBits(visibleCells[pos[c]]);
					goto next_pos;
				}
			}
			//this cell has been just cleared by setting other naked single
			mode = MODE_STOP_PROCESSING;
			return;
next_pos:
			;
		}
		grid[0].clearBits(all);
		grid[1].clearBits(all);
		grid[2].clearBits(all);
		grid[3].clearBits(all);
		grid[4].clearBits(all);
		grid[5].clearBits(all);
		grid[6].clearBits(all);
		grid[7].clearBits(all);
		grid[8].clearBits(all);
		solved |= all;
		if(((bm128)maskLSB[81]).isSubsetOf(solved)) {
			solutionFound();
			return;
		}
	} while(1);
}

void fsss2::doHiddenSingles() { //digits with only one occurrence in a house
#pragma forceinline recursive
	for(int d = 0; d < 9; d++) { //for each digit
		doHiddenSinglesForDigit(d);
		if(mode) return;
	}
//	doHiddenSinglesForDigit(0);
//	if(mode) return;
//	doHiddenSinglesForDigit(1);
//	if(mode) return;
//	doHiddenSinglesForDigit(2);
//	if(mode) return;
//	doHiddenSinglesForDigit(3);
//	if(mode) return;
//	doHiddenSinglesForDigit(4);
//	if(mode) return;
//	doHiddenSinglesForDigit(5);
//	if(mode) return;
//	doHiddenSinglesForDigit(6);
//	if(mode) return;
//	doHiddenSinglesForDigit(7);
//	if(mode) return;
//	doHiddenSinglesForDigit(8);
}

void fsss2::doHiddenSinglesForDigit(int d) { //digits with only one occurrence in a house
	again:
	//bool found = false;
	//get unsolved houses
	//int houses = ((1 << 27) - 1) & (grid[d].toInt64_1()) >> (81 - 64);
	int houses = (grid[d].toInt64_1()) >> (81 - 64);
	for(int hbm = houses & -houses; houses; hbm = houses & -houses) {
		houses ^= hbm;
		unsigned int h = __builtin_ctz(hbm);
		bm128 tmp = grid[d];
		tmp &= bitsForHouse[h];
		int n = tmp.findSingleBitIndex96();
		if(n == -1) // 2+ bits
			continue;
		if(n == -2) { // 0 bits
			mode = MODE_STOP_PROCESSING;
			return;
		}
		setDigit(d, n);
		if(mode) return;
		//found = true;
		doNakedSingles(); //parallel version looks fast enough even for checking a single cell
		if(mode) return;
		goto again;
		//houses &= (grid[d].toInt64_1()) >> (81 - 64); //clear the possibly solved houses
	}
//	if(found) {
//		//doNakedSingles(); //parallel version looks fast enough even for checking a single cell
//		//if(mode) return;
//		goto again;
//	}
}

//void fsss2::doHiddenSinglesForDigitCell(int d, int c) { //digits with only one occurrence in a house
//	static const int affectedGroups[81][3] =
//	{
//		{0, 9,18},{0,10,18},{0,11,18},{0,12,19},{0,13,19},{0,14,19},{0,15,20},{0,16,20},{0,17,20},
//		{1, 9,18},{1,10,18},{1,11,18},{1,12,19},{1,13,19},{1,14,19},{1,15,20},{1,16,20},{1,17,20},
//		{2, 9,18},{2,10,18},{2,11,18},{2,12,19},{2,13,19},{2,14,19},{2,15,20},{2,16,20},{2,17,20},
//		{3, 9,21},{3,10,21},{3,11,21},{3,12,22},{3,13,22},{3,14,22},{3,15,23},{3,16,23},{3,17,23},
//		{4, 9,21},{4,10,21},{4,11,21},{4,12,22},{4,13,22},{4,14,22},{4,15,23},{4,16,23},{4,17,23},
//		{5, 9,21},{5,10,21},{5,11,21},{5,12,22},{5,13,22},{5,14,22},{5,15,23},{5,16,23},{5,17,23},
//		{6, 9,24},{6,10,24},{6,11,24},{6,12,25},{6,13,25},{6,14,25},{6,15,26},{6,16,26},{6,17,26},
//		{7, 9,24},{7,10,24},{7,11,24},{7,12,25},{7,13,25},{7,14,25},{7,15,26},{7,16,26},{7,17,26},
//		{8, 9,24},{8,10,24},{8,11,24},{8,12,25},{8,13,25},{8,14,25},{8,15,26},{8,16,26},{8,17,26}
//	};
//	for(int h = 0; h < 3; h++) {
//		bm128 tmp = grid[d];
//		if(!tmp.isBitSet(81 + affectedGroups[c][h]))
//			continue;
//		tmp &= bitsForHouse[affectedGroups[c][h]];
//		int n = tmp.findSingleBitIndex96();
//		if(n == -1) // 2+ bits
//			continue;
//		if(n == -2) { // 0 bits
//			mode = MODE_STOP_PROCESSING;
//			return;
//		}
//		setDigit(d, n);
//		if(mode) return;
//		//doNakedSingles(); //parallel version looks fast enough even for checking a single cell
//		//if(mode) return;
//	}
//}

//void fsss2::doLockedCandidatesForCell(int d, int c) { //after elimination
//	static const unsigned int rowByCellIndex[81] =
//	{
//		0,0,0,0,0,0,0,0,0, 1,1,1,1,1,1,1,1,1, 2,2,2,2,2,2,2,2,2,
//		3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4, 5,5,5,5,5,5,5,5,5,
//		6,6,6,6,6,6,6,6,6, 7,7,7,7,7,7,7,7,7, 8,8,8,8,8,8,8,8,8
//	};
//	static const unsigned int colByCellIndex[81] =
//	{
//		0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8,
//		0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8,
//		0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8, 0,1,2,3,4,5,6,7,8
//	};
//	static const unsigned int bandByCellIndex[81] =
//	{
//		0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,
//		1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,
//		2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2
//	};
//	static const unsigned int stackByCellIndex[81] =
//	{
//		0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2,
//		0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2,
//		0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2, 0,0,0,1,1,1,2,2,2
//	};
//	bm128 tmp = grid[d];
//	unsigned int rc;
//	rc = 3 * rowByCellIndex[c] + stackByCellIndex[c];
//	if(!tmp.isDisjoint(tripletMasks[rc].self)) {
//		if(tmp.isDisjoint(tripletMasks[rc].adjacentLine)) {
//			//value is within this triplet, therefore it isn't in the rest 2 triplets in the box
//			tmp.clearBits(tripletMasks[rc].adjacentBox);
//		}
//		else if(tmp.isDisjoint(tripletMasks[rc].adjacentBox)) {
//			//value is within this triplet, therefore it isn't in the rest 2 triplets in the row/col
//			tmp.clearBits(tripletMasks[rc].adjacentLine);
//		}
//	}
//	rc = 27 + 3 * colByCellIndex[c] + bandByCellIndex[c];
//	if(!tmp.isDisjoint(tripletMasks[rc].self)) {
//		if(tmp.isDisjoint(tripletMasks[rc].adjacentLine)) {
//			//value is within this triplet, therefore it isn't in the rest 2 triplets in the box
//			tmp.clearBits(tripletMasks[rc].adjacentBox);
//		}
//		else if(tmp.isDisjoint(tripletMasks[rc].adjacentBox)) {
//			//value is within this triplet, therefore it isn't in the rest 2 triplets in the row/col
//			tmp.clearBits(tripletMasks[rc].adjacentLine);
//		}
//	}
//	if(!grid[d].isSubsetOf(tmp)) { //some eliminations are ready for this digit
//		grid[d] = tmp;
//		//doNakedSingles();
//		//if(mode) return;
//		//doHiddenSinglesForDigit(d);
//	}
//}

void fsss2::doLockedCandidatesForDigit(bm128& tmp) {
	//bm128 tmp = grid[d];
	int houses = 0x03FFFF & ((tmp.toInt64_1()) >> (81 - 64));
	for(int hbm = houses & -houses; houses; hbm = houses & -houses) {
		houses ^= hbm;
		unsigned int rc = 3U * __builtin_ctz(hbm);
		//process the 3 triplets in the row/col
		//at least one of them is joint to grid[d], i.e. is not solved
		if(!tmp.isDisjoint(tripletMasks[rc + 0].self)) {
			if(tmp.isDisjoint(tripletMasks[rc + 0].adjacentLine)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the box
				tmp.clearBits(tripletMasks[rc + 0].adjacentBox);
			}
			else if(tmp.isDisjoint(tripletMasks[rc + 0].adjacentBox)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the row/col
				tmp.clearBits(tripletMasks[rc + 0].adjacentLine);
			}
		}
		if(!tmp.isDisjoint(tripletMasks[rc + 1].self)) {
			if(tmp.isDisjoint(tripletMasks[rc + 1].adjacentLine)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the box
				tmp.clearBits(tripletMasks[rc + 1].adjacentBox);
			}
			else if(tmp.isDisjoint(tripletMasks[rc + 1].adjacentBox)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the row/col
				tmp.clearBits(tripletMasks[rc + 1].adjacentLine);
			}
		}
		if(!tmp.isDisjoint(tripletMasks[rc + 2].self)) {
			if(tmp.isDisjoint(tripletMasks[rc + 2].adjacentLine)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the box
				tmp.clearBits(tripletMasks[rc + 2].adjacentBox);
			}
			else if(tmp.isDisjoint(tripletMasks[rc + 2].adjacentBox)) {
				//value is within this triplet, therefore it isn't in the rest 2 triplets in the row/col
				tmp.clearBits(tripletMasks[rc + 2].adjacentLine);
			}
		}
	} //houses loop
}

void fsss2::doLockedCandidates() { //if a digit in a row is within a single triplet, then remove digit from the box triplets and vice versa
	if(lockedCandidatesDone) return;
again:
//#pragma forceinline recursive
	for(int d = 0; d < 9; d++) {
		bm128 tmp = grid[d];
		doLockedCandidatesForDigit(grid[d]);
		if(tmp.isSubsetOf(grid[d]))
			continue;
		//some eliminations are done for this digit
		doNakedSingles();
		if(mode) return;
		doHiddenSinglesForDigit(d);
		if(mode) return;
		//goto again;
		//lockedCandidatesDone = 1;
	}
	//if(	lockedCandidatesDone)
	//	doHiddenSingles();
	lockedCandidatesDone = 1;
}

void fsss2::doDirectEliminations() {
again:
	doNakedSingles();
	if(mode) return;
	doHiddenSingles();
	//if(mode) return;
	//doLockedCandidates();
}

void fsss2::doEliminations() {
	doDirectEliminations();
	if(mode) return;
	guess();
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
	all &= maskLSB[81]; //clear other bits
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
	//return;
	//Prepare a guess
again:
	//Find an unsolved cell with less possibilities
	int optDigit;
	int optDigit2;
	int optCell;
	int minCells = 100;
	int n;

	findBiValueCell(optDigit, optCell, optDigit2);
	if(optDigit != -1) {
		guess1(optDigit, optCell);
		if(mode) return;
		//we have in hands a secondary digit to set
		setDigit(optDigit2, optCell);
		if(mode) return;
	}
	else {
		//biposition value
		for(int d = 0; d < 9; d++) {
			for(int h = 0; h < 27; h++) {
				if(!grid[d].isBitSet(81 + h))
					continue;
				bm128 tmp = grid[d];
				tmp &= bitsForHouse[h];
				if(tmp.isZero()) {
					continue;
				}
				tmp &= maskLSB[81];
				n = tmp.popcount_128();
				if(n < minCells) {
					optDigit = d;
					optCell = tmp.getFirstBit1Index96();
					if(optCell < 0) {
						return;
					}
					if(n == 2)
						break;
					minCells = n;
				}
			}
		}
//		if(optDigit == -1) {
//			mode = MODE_STOP_PROCESSING;
//			return; //shouldn't happen
//		}
		guess1(optDigit, optCell);
		if(mode) return;
		grid[optDigit].clearBit(optCell);
		//doHiddenSinglesForDigit(optDigit);
		//doHiddenSinglesForDigitCell(optDigit, optCell);
		//doLockedCandidatesForCell(digit, cell);
		//doLockedCandidatesForDigit(optDigit);
		//if(mode) return;
	}
	doDirectEliminations();
	if(mode) return;
	goto again;
}

void fsss2::guess1(int digit, int cell) {
	fsss2 gg = *this; //copy the solution context
	setDigit(digit, cell);
	if(mode) return;
	doEliminations();
	if(mode & MODE_STOP_GUESSING) {
		return;
	}
	//We are done with the guess.
	//The caller is notified for each of the the possible solutions found so far
	//Now restore the context and later remove this candidate from the further solutions or set the only remaining candidate
	*this = gg;
}

