/*
 *
 *  Created on: May 13, 2014
 *      Author: Mladen Dobrichev
 */

//Fast Simple Sudoku Solver 2

#include <stdio.h>
#include <memory.h>
#include "fsss2.h"

//#define COUNT_TRIALS

//game mode flags
#define MODE_SOLVING			0	//unused, keep solving
#define MODE_STOP_PROCESSING	1	//solved or error
#define MODE_STOP_GUESSING		2	//necessary solutions found

#ifdef COUNT_TRIALS
extern int nTrials;
#endif

template <class X> fsss2<X>::fsss2(X &theCollector) : collector(theCollector) {};

//only first 81 bits set
const t_128 constraints::mask81 = {0xFFFFFFFFFFFFFFFF,    0x0001FFFF};

//only first 81 + 27 = 108 bits set
//template <class X> const t_128 fsss2<X>::mask108 = {0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFF};

//the first 81 bits for the cells and the 27 bits from position 96+
const t_128 constraints::mask108 = {0xFFFFFFFFFFFFFFFF,0x07FFFFFF0001FFFF};

//the 27 bits from position 96+
const t_128 constraints::mask27 = {0x0,0x07FFFFFF00000000};

const t_128 constraints::visibleCells[81] = { //1 for all 20 visible cells, 1 for the cell itself, 1 for the three houses
	{0x80402010081C0FFF,0x0004020100000100},
	{0x00804020101C0FFF,0x0004040100000201},
	{0x01008040201C0FFF,0x0004080100000402},
	{0x0201008040E071FF,0x0008100100000804},
	{0x0402010080E071FF,0x0008200100001008},
	{0x0804020100E071FF,0x0008400100002010},
	{0x10080402070381FF,0x0010800100004020},
	{0x20100804070381FF,0x0011000100008040},
	{0x40201008070381FF,0x0012000100010080},
	{0x80402010081FFE07,0x0004020200000100},
	{0x00804020101FFE07,0x0004040200000201},
	{0x01008040201FFE07,0x0004080200000402},
	{0x0201008040E3FE38,0x0008100200000804},
	{0x0402010080E3FE38,0x0008200200001008},
	{0x0804020100E3FE38,0x0008400200002010},
	{0x100804020703FFC0,0x0010800200004020},
	{0x201008040703FFC0,0x0011000200008040},
	{0x402010080703FFC0,0x0012000200010080},
	{0x804020100FFC0E07,0x0004020400000100},
	{0x0080402017FC0E07,0x0004040400000201},
	{0x0100804027FC0E07,0x0004080400000402},
	{0x0201008047FC7038,0x0008100400000804},
	{0x0402010087FC7038,0x0008200400001008},
	{0x0804020107FC7038,0x0008400400002010},
	{0x1008040207FF81C0,0x0010800400004020},
	{0x2010080407FF81C0,0x0011000400008040},
	{0x4020100807FF81C0,0x0012000400010080},
	{0x8040E07FF8040201,0x0020020800000100},
	{0x0080E07FF8080402,0x0020040800000201},
	{0x0100E07FF8100804,0x0020080800000402},
	{0x0207038FF8201008,0x0040100800000804},
	{0x0407038FF8402010,0x0040200800001008},
	{0x0807038FF8804020,0x0040400800002010},
	{0x10381C0FF9008040,0x0080800800004020},
	{0x20381C0FFA010080,0x0081000800008040},
	{0x40381C0FFC020100,0x0082000800010080},
	{0x8040FFF038040201,0x0020021000000100},
	{0x0080FFF038080402,0x0020041000000201},
	{0x0100FFF038100804,0x0020081000000402},
	{0x02071FF1C0201008,0x0040101000000804},
	{0x04071FF1C0402010,0x0040201000001008},
	{0x08071FF1C0804020,0x0040401000002010},
	{0x10381FFE01008040,0x0080801000004020},
	{0x20381FFE02010080,0x0081001000008040},
	{0x40381FFE04020100,0x0082001000010080},
	{0x807FE07038040201,0x0020022000000100},
	{0x00BFE07038080402,0x0020042000000201},
	{0x013FE07038100804,0x0020082000000402},
	{0x023FE381C0201008,0x0040102000000804},
	{0x043FE381C0402010,0x0040202000001008},
	{0x083FE381C0804020,0x0040402000002010},
	{0x103FFC0E01008040,0x0080802000004020},
	{0x203FFC0E02010080,0x0081002000008040},
	{0x403FFC0E04020100,0x0082002000010080},
	{0xFFC0201008040201,0x0100024000000703},
	{0xFFC0402010080402,0x0100044000000703},
	{0xFFC0804020100804,0x0100084000000703},
	{0x7FC1008040201008,0x020010400000381C},
	{0x7FC2010080402010,0x020020400000381C},
	{0x7FC4020100804020,0x020040400000381C},
	{0x7FC8040201008040,0x040080400001C0E0},
	{0x7FD0080402010080,0x040100400001C0E0},
	{0x7FE0100804020100,0x040200400001C0E0},
	{0x81C0201008040201,0x01000280000007FF},
	{0x81C0402010080402,0x01000480000007FF},
	{0x81C0804020100804,0x01000880000007FF},
	{0x8E01008040201008,0x02001080000038FF},
	{0x8E02010080402010,0x02002080000038FF},
	{0x8E04020100804020,0x02004080000038FF},
	{0xF008040201008040,0x040080800001C0FF},
	{0xF010080402010080,0x040100800001C0FF},
	{0xF020100804020100,0x040200800001C0FF},
	{0x81C0201008040201,0x010003000001FF03},
	{0x81C0402010080402,0x010005000001FF03},
	{0x81C0804020100804,0x010009000001FF03},
	{0x0E01008040201008,0x020011000001FF1C},
	{0x0E02010080402010,0x020021000001FF1C},
	{0x0E04020100804020,0x020041000001FF1C},
	{0x7008040201008040,0x040081000001FFE0},
	{0x7010080402010080,0x040101000001FFE0},
	{0x7020100804020100,0x040201000001FFE0}
}; //bm128 visibleCells[81]

const t_128 constraints::bitsForHouse[27] = { //1 for the 9 cells in the house
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

//const uint32 constraints::topCellsHouses = 0x00FC007F; //000111111000000000001111111

#ifdef USE_LOCKED_CANDIDATES
const tripletMask constraints::tripletMasks[54] = {
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
#endif

template <class X> void fsss2<X>::initEmpty() {
	//set all cells and houses as "unsolved"
	grid[0] = constraints::mask108;
	grid[1] = constraints::mask108;
	grid[2] = constraints::mask108;
	grid[3] = constraints::mask108;
	grid[4] = constraints::mask108;
	grid[5] = constraints::mask108;
	grid[6] = constraints::mask108;
	grid[7] = constraints::mask108;
	grid[8] = constraints::mask108;
	//no processed digits for hidden singles yet
	knownNoHiddenSingles[0] = constraints::mask108;
	knownNoHiddenSingles[1] = constraints::mask108;
	knownNoHiddenSingles[2] = constraints::mask108;
	knownNoHiddenSingles[3] = constraints::mask108;
	knownNoHiddenSingles[4] = constraints::mask108;
	knownNoHiddenSingles[5] = constraints::mask108;
	knownNoHiddenSingles[6] = constraints::mask108;
	knownNoHiddenSingles[7] = constraints::mask108;
	knownNoHiddenSingles[8] = constraints::mask108;

#ifdef USE_LOCKED_CANDIDATES
//	knownNoLockedCandidates[0] = constraints::mask108;
//	knownNoLockedCandidates[1] = constraints::mask108;
//	knownNoLockedCandidates[2] = constraints::mask108;
//	knownNoLockedCandidates[3] = constraints::mask108;
//	knownNoLockedCandidates[4] = constraints::mask108;
//	knownNoLockedCandidates[5] = constraints::mask108;
//	knownNoLockedCandidates[6] = constraints::mask108;
//	knownNoLockedCandidates[7] = constraints::mask108;
//	knownNoLockedCandidates[8] = constraints::mask108;
	lockedDone = 0;
#endif
#ifdef USE_SUBSETS
	subsetsDone = 0;
//	for(int i = 0; i < 36; i++) knownNoSubsets[i] = constraints::mask108;
#endif //USE_SUBSETS
	solved.clear(); //no solved cells yet
	trialCandidates.clear(); //unknown cell candidates for T&E
	mode = 0; //should solve
	guessDepth = 0; //no guessed cells yet
}

template <class X> void fsss2<X>::setCellValue(int pos, int value) {
	collector.setCellValue(pos, value);
	solved.setBit(pos); //mark cell as "solved"
	grid[value - 1].clearBits(constraints::visibleCells[pos]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
}

template <class X> void fsss2<X>::eliminateCellValue(int pos, int value) {
	grid[value - 1].clearBit(pos);
}

#ifdef USE_LOCKED_CANDIDATES
template <class X> bool fsss2<X>::doLockedCandidatesForDigit(bm128& tmp) {
	bool found = false;
	for(uint32_t lines = 0x03FFFF & (tmp.toInt32_3()); lines; lines &= (lines - 1)) {
		unsigned int line = 3U * bm128::FindLSBIndex32(lines); //unsolved row or column
		//process the 3 triplets in the line (row or column)
		for(unsigned int t = 0; t < 3; t++) {
			bool dl = tmp.isDisjoint(constraints::tripletMasks[line + t].adjacentLine);
			bool db = tmp.isDisjoint(constraints::tripletMasks[line + t].adjacentBox);
			if(dl) {
				//unsolved line with candidates located only within this triplet
				if(!db) {
					tmp.clearBits(constraints::tripletMasks[line + t].adjacentBox);
					found = true;
					goto next_line;
					//return true;
				}
			}
			if(db) {
				//the other two triplets within this box have no candidates
				if(!dl) {
					//other two triplets for this line have candidates, but the box is possibly solved
					if(!tmp.isDisjoint(constraints::tripletMasks[line + t].self)) { //unsolved box
						tmp.clearBits(constraints::tripletMasks[line + t].adjacentLine);
						found = true;
						goto next_line;
						//return true;
					}
				}
			}
		} //triplets in a line
next_line:
		;
	} //lines
	return found;
	//return false;
}
#endif

//void fsss2::findBiValueCells(bm128& all) const { //cells with 2 remaining candidates
//	//bm128 all;
//	all = solved;
//	bm128 duplicates = solved;
//	bm128 triplicates = solved;
//	for(int d = 0; d < 9; d++) {
//		bm128 tmp = grid[d];
//		tmp &= all;
//		bm128 tmp2 = tmp;
//		tmp2 &= duplicates;
//		triplicates |= tmp2;
//		duplicates |= tmp;
//		all |= grid[d];
//	}
//	all &= constraints::mask81; //clear other bits
//	all.clearBits(triplicates);
//}
template <class X> int fsss2<X>::findLeastPopulatedCells(bm128& all) const {
	//the following code is written by a member of http://forum.enjoysudoku.com known by pseudonym Blue
	//it returns the cells with less number of candidates and works for 0 to 9 candidates (4-bit sum)
   bm128 sum0 = grid[0];

   bm128 sum1 = grid[1];
   bm128 tmp = sum0;
   sum0 ^= sum1;
   sum1 &= tmp;

   bm128 carry = grid[2];
   tmp = sum0;
   sum0 ^= carry;
   carry &= tmp;
   sum1 |= carry;

   bm128 sum2 = grid[3];
   tmp = sum0;
   sum0 ^= sum2;
   sum2 &= tmp;
   tmp = sum1;
   sum1 ^= sum2;
   sum2 &= tmp;

   carry = grid[4];
   tmp = sum0;
   sum0 ^= carry;
   carry &= tmp;
   tmp = sum1;
   sum1 ^= carry;
   carry &= tmp;
   sum2 |= carry;

   carry = grid[5];
   tmp = sum0;
   sum0 ^= carry;
   carry &= tmp;
   tmp = sum1;
   sum1 ^= carry;
   carry &= tmp;
   sum2 |= carry;

   carry = grid[6];
   tmp = sum0;
   sum0 ^= carry;
   carry &= tmp;
   tmp = sum1;
   sum1 ^= carry;
   carry &= tmp;
   sum2 |= carry;

   bm128 sum3 = grid[7];
   tmp = sum0;
   sum0 ^= sum3;
   sum3 &= tmp;
   tmp = sum1;
   sum1 ^= sum3;
   sum3 &= tmp;
   tmp = sum2;
   sum2 ^= sum3;
   sum3 &= tmp;

   carry = grid[8];
   tmp = sum0;
   sum0 ^= carry;
   carry &= tmp;
   tmp = sum1;
   sum1 ^= carry;
   carry &= tmp;
   tmp = sum2;
   sum2 ^= carry;
   carry &= tmp;
   sum3 |= carry;

   all = constraints::mask81;
   all.clearBits(solved);

   int n = 0;
   if (all.isSubsetOf(sum3)) n = 8;  else all.clearBits(sum3);
   if (all.isSubsetOf(sum2)) n += 4; else all.clearBits(sum2);
   if (all.isSubsetOf(sum1)) n += 2; else all.clearBits(sum1);
   if (all.isSubsetOf(sum0)) n++;    else all.clearBits(sum0);
   return n;
}

#if 0
bool inline clearNaked(bm128 &gn, bm128 &mask) {
	bm128 theCells = mask; theCells &= gn;
	bool x = false;
	for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
		uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
		if(x && !gn.isBitSet(cell)) return true;
		x = true;
		gn.clearBits(constraints::visibleCells[cell]);
		//collector.setCellValue(cell, 1);
	} //for lower 64 cells
	for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
		uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
		if(x && !gn.isBitSet(cell)) return true;
		x = true;
		gn.clearBits(constraints::visibleCells[cell]);
		//collector.setCellValue(cell, 1);
	}
	mask.clearBits(theCells);
	return false;
};
#endif
#if 1
bool inline clearNaked(bm128 &gn, bm128 &mask) {
	bm128 theCells = mask; theCells &= gn;
	uint32_t solvedHouses = 0;
	for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
		uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
		if(constraints::visibleCells[cell].m128i_u32[3] & solvedHouses) return true;
		solvedHouses |= constraints::visibleCells[cell].m128i_u32[3];
		gn.clearBits(constraints::visibleCells[cell]);
		//collector.setCellValue(cell, 1);
	} //for lower 64 cells
	for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
		uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
		if(constraints::visibleCells[cell].m128i_u32[3] & solvedHouses) return true;
		solvedHouses |= constraints::visibleCells[cell].m128i_u32[3];
		gn.clearBits(constraints::visibleCells[cell]);
		//collector.setCellValue(cell, 1);
	}
	mask.clearBits(theCells);
	return false;
};
#endif

template <class X> void fsss2<X>::doNakedSingles() { //cells with only one remaining candidate
//    __asm__
//    (
//			"":::"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7","%xmm8","%xmm9","%xmm10","%xmm11","%xmm12","xmm13","xmm14","xmm15"
//    );
#if 1
    register bm128 g0 = grid[0];
	register bm128 g1 = grid[1];
	register bm128 g2 = grid[2];
	register bm128 g3 = grid[3];
	register bm128 g4 = grid[4];
	register bm128 g5 = grid[5];
	register bm128 g6 = grid[6];
	register bm128 g7 = grid[7];
	register bm128 g8 = grid[8];
#else
	#define g0 (grid[0])
	#define g1 (grid[1])
	#define g2 (grid[2])
	#define g3 (grid[3])
	#define g4 (grid[4])
	#define g5 (grid[5])
	#define g6 (grid[6])
	#define g7 (grid[7])
	#define g8 (grid[8])
#endif
    bm128& slv = solved;
	do {
		register bm128 all = slv;
		{
			register bm128 duplicates = slv; //cells with 2 or more candidates
			{bm128 tmp = g0; tmp &= all; duplicates |= tmp; all |= g0;}
			{bm128 tmp = g1; tmp &= all; duplicates |= tmp; all |= g1;}
			{bm128 tmp = g2; tmp &= all; duplicates |= tmp; all |= g2;}
			{bm128 tmp = g3; tmp &= all; duplicates |= tmp; all |= g3;}
			{bm128 tmp = g4; tmp &= all; duplicates |= tmp; all |= g4;}
			{bm128 tmp = g5; tmp &= all; duplicates |= tmp; all |= g5;}
			{bm128 tmp = g6; tmp &= all; duplicates |= tmp; all |= g6;}
			{bm128 tmp = g7; tmp &= all; duplicates |= tmp; all |= g7;}
			{bm128 tmp = g8; tmp &= all; duplicates |= tmp; all |= g8;}
			if(((bm128)constraints::mask81).isSubsetOf(all)) {
				;
			}
			else {
				//there is at least one unsolved cell without any candidate
				mode = MODE_STOP_PROCESSING;
				return;
			}
			if(((bm128)constraints::mask81).isSubsetOf(duplicates)) {
				//sorry, no naked singles
				//remove all candidates for the solved cells (even if no eliminations were made here)
				//Note: this cleanup is postponed up to there because
				// - the above checking is not sensitive to this aspect of the board consistency, and
				// - there is a chance contradiction to be detected before the cleanup
				g0.clearBits(slv);
				g1.clearBits(slv);
				g2.clearBits(slv);
				g3.clearBits(slv);
				g4.clearBits(slv);
				g5.clearBits(slv);
				g6.clearBits(slv);
				g7.clearBits(slv);
				g8.clearBits(slv);
				grid[0] = g0;
				grid[1] = g1;
				grid[2] = g2;
				grid[3] = g3;
				grid[4] = g4;
				grid[5] = g5;
				grid[6] = g6;
				grid[7] = g7;
				grid[8] = g8;
				return;
			}
			all = constraints::mask81;
			all.clearBits(duplicates);
			slv |= all; //mark cells as solved
		}
		//now find which unique where came from
#if 0
		for(uint64_t cells = all.toInt64(); cells; cells &= (cells - 1)) {
			uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
			const bm128 theCells = constraints::visibleCells[cell];
			const bm128 theBit = bitSet[cell];
			if(!theBit.isSubsetOf(g0)) {;} else {g0.clearBits(theCells); collector.setCellValue(cell, 1); continue;}
			if(!theBit.isSubsetOf(g1)) {;} else {g1.clearBits(theCells); collector.setCellValue(cell, 2); continue;}
			if(!theBit.isSubsetOf(g2)) {;} else {g2.clearBits(theCells); collector.setCellValue(cell, 3); continue;}
			if(!theBit.isSubsetOf(g3)) {;} else {g3.clearBits(theCells); collector.setCellValue(cell, 4); continue;}
			if(!theBit.isSubsetOf(g4)) {;} else {g4.clearBits(theCells); collector.setCellValue(cell, 5); continue;}
			if(!theBit.isSubsetOf(g5)) {;} else {g5.clearBits(theCells); collector.setCellValue(cell, 6); continue;}
			if(!theBit.isSubsetOf(g6)) {;} else {g6.clearBits(theCells); collector.setCellValue(cell, 7); continue;}
			if(!theBit.isSubsetOf(g7)) {;} else {g7.clearBits(theCells); collector.setCellValue(cell, 8); continue;}
			if(!theBit.isSubsetOf(g8)) {;} else {g8.clearBits(theCells); collector.setCellValue(cell, 9); continue;}
			//this cell has been just cleared by setting other naked single (2 naked in a house for the same digit)
			//now this cell has no candidates which is a contradiction
			mode = MODE_STOP_PROCESSING;
			return;
		} //for lower 64 cells
		for(uint32_t cells = all.toInt32_2(); cells; cells &= (cells - 1)) {
			uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
			const bm128 theCells = constraints::visibleCells[cell];
			const bm128 theBit = bitSet[cell];
			if(!theBit.isSubsetOf(g0)) {;} else {g0.clearBits(theCells); collector.setCellValue(cell, 1); continue;}
			if(!theBit.isSubsetOf(g1)) {;} else {g1.clearBits(theCells); collector.setCellValue(cell, 2); continue;}
			if(!theBit.isSubsetOf(g2)) {;} else {g2.clearBits(theCells); collector.setCellValue(cell, 3); continue;}
			if(!theBit.isSubsetOf(g3)) {;} else {g3.clearBits(theCells); collector.setCellValue(cell, 4); continue;}
			if(!theBit.isSubsetOf(g4)) {;} else {g4.clearBits(theCells); collector.setCellValue(cell, 5); continue;}
			if(!theBit.isSubsetOf(g5)) {;} else {g5.clearBits(theCells); collector.setCellValue(cell, 6); continue;}
			if(!theBit.isSubsetOf(g6)) {;} else {g6.clearBits(theCells); collector.setCellValue(cell, 7); continue;}
			if(!theBit.isSubsetOf(g7)) {;} else {g7.clearBits(theCells); collector.setCellValue(cell, 8); continue;}
			if(!theBit.isSubsetOf(g8)) {;} else {g8.clearBits(theCells); collector.setCellValue(cell, 9); continue;}
			//this cell has been just cleared by setting other naked single (2 naked in a house for the same digit)
			//now this cell has no candidates which is a contradiction
			mode = MODE_STOP_PROCESSING;
			return;
		} //for upper 17 cells
#elif 0
		if(!g8.isDisjoint(all)) {if(clearNaked(g8, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g7.isDisjoint(all)) {if(clearNaked(g7, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g6.isDisjoint(all)) {if(clearNaked(g6, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g5.isDisjoint(all)) {if(clearNaked(g5, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g4.isDisjoint(all)) {if(clearNaked(g4, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g3.isDisjoint(all)) {if(clearNaked(g3, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g2.isDisjoint(all)) {if(clearNaked(g2, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g1.isDisjoint(all)) {if(clearNaked(g1, all)) goto contradiction; if(all.isZero()) continue;}
		if(!g0.isDisjoint(all)) {if(clearNaked(g0, all)) goto contradiction; if(all.isZero()) continue;}
		printf(".");
#else
		if(!g0.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g0;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g0.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g0.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 1);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g0.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g0.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 1);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g1.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g1;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g1.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g1.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 2);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g1.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g1.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 2);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g2.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g2;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g2.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g2.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 3);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g2.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g2.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 3);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g3.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g3;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g3.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g3.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 4);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g3.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g3.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 4);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g4.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g4;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g4.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g4.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 5);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g4.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g4.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 5);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g5.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g5;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g5.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g5.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 6);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g5.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g5.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 6);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g6.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g6;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g6.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g6.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 7);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g6.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g6.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 7);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		if(!g7.isDisjoint(all)) {
			bm128 theCells = all; theCells &= g7;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g7.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g7.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 8);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g7.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g7.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 8);
			}
			if(all.isSubsetOf(theCells)) continue;
			all.clearBits(theCells);
		}
		{
			bm128 theCells = all; theCells &= g8;
			for(uint64_t cells = theCells.toInt64(); cells; cells &= (cells - 1)) {
				uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
				if(!g8.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g8.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 9);
			} //for lower 64 cells
			for(uint32_t cells = theCells.toInt32_2(); cells; cells &= (cells - 1)) {
				uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
				if(!g8.isBitSet(cell)) goto contradiction; //two naked singles for the same digit in the same house
				g8.clearBits(constraints::visibleCells[cell]);
				collector.setCellValue(cell, 9);
			}
		}
#endif
	} while(!((bm128)constraints::mask81).isSubsetOf(slv));
	//finally all 81 cells are solved
	if(collector.solutionFound()) {
		//collector doesn't ask for more solutions, we are done
		mode = MODE_STOP_PROCESSING | MODE_STOP_GUESSING;
		return;
	}
	//continue with next possible solution

contradiction:
	mode = MODE_STOP_PROCESSING;
	return;
}

template <class X> void fsss2<X>::doEliminations() {
nakedAgain:
	doNakedSingles();
	if(mode) goto backtrack;

	//hidden singles
	//if(0)
	{
		int found;
		do {
			found = 0;
			for(int d = 0; d < 9; d++) { //for each digit
				if(knownNoHiddenSingles[d] == grid[d]) {
					continue;
				}
againSameHidden:
				//for each unsolved house
				for(uint32_t houses = grid[d].toInt32_3(); houses; houses &= (houses - 1)) {
					uint32_t house = bm128::FindLSBIndex32(houses);
					bm128 tmp = grid[d];
					tmp &= constraints::bitsForHouse[house]; //mask other candidates and leave only these from the current house

					//find whether the house has a single candidate and obtain its position
					//exploit the fact that when (x & (x-1)) == 0 then x has 0 or 1 bits set
					if(tmp.hasMin2Bits())
						continue; //too many candidates

					//find the bit
					int cell;
					{
						uint64_t low64 = tmp.toInt64();
						uint32_t high17 = tmp.toInt32_2();
						if(low64) {
							if(high17)
								continue; //candidates in both low and high part of the house
							//get the position of the single candidate in the low part of the house
							cell = (int) bm128::FindLSBIndex64(low64);
							goto single_found;
						}
						if(high17) {
							//get the position of the single candidate in the high part of the house
							cell = 64 + bm128::FindLSBIndex32(high17);
							goto single_found;
						}
					}
					//no any candidate for this digit in this unsolved house
					goto contradiction;
single_found:
					collector.setCellValue(cell, d + 1);
					solved.setBit(cell); //mark cell as "solved"
					grid[d].clearBits(constraints::visibleCells[cell]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
					//at this point the solved cell still isn't cleared from all 9 masks, but doNakedSingles does it
					doNakedSingles(); //checking for a single candidate in a cell, possible eliminations in other digits
					if(mode) goto backtrack;
					found = d; //if the latest found is in the 1-st digit d == 0, then repeating search for hidden singles is redundant
					goto againSameHidden;
				} //for houses
				knownNoHiddenSingles[d] = grid[d];
			}  //for d
		} while(found);
	} //end of hidden singles

	//locked candidates
#ifdef USE_LOCKED_CANDIDATES
	if(lockedDone == 0) {
		bool found = false;
		//if a digit in a row is within a single triplet, then remove digit from the other two box triplets and vice versa
		for (int d = 0; d < 9; d++) {
			found |= doLockedCandidatesForDigit(grid[d]);
//			if(knownNoLockedCandidates[d] == grid[d])
//				continue;
//			if(doLockedCandidatesForDigit(grid[d])) {
//				found = true;
//			}
//			else {
//				knownNoLockedCandidates[d] = grid[d];
//			}
		}
		lockedDone = 1;
		if(found) {
#ifdef USE_SUBSETS
			//subsetsDone = 0;
#endif //USE_SUBSETS
			goto nakedAgain;
		}
		//lockedDone = 1;
	} //end of locked candidates
#endif //USE_LOCKED_CANDIDATES

#ifdef USE_SUBSETS
	//subsets
	if(subsetsDone == 0) {
	//if(subsetsDone < 5) {
		bool eliminationFound = false;
		//int subsetIndex = 0; //01,02,03,04,05,06,07,08,12,13,14,15,16,17,18,23,24,25,26,27,28,34,35,36,37,38,45,46,47,48,56,57,58,67,68,78
		for(int d1 = 0; d1 < 8; d1++) {
			//for(int d2 = d1 + 1; d2 < 9; d2++, subsetIndex++) {
			for(int d2 = d1 + 1; d2 < 9; d2++) {
				bm128 any = grid[d1];
				any |= grid[d2];
//				if(any == knownNoSubsets[subsetIndex])
//					continue;
				bool pairFound = false;
				bm128 ss;
				ss.clear();
				bm128 both = grid[d1];
				both &= grid[d2];
				//for each unsolved for both digits house
				for(uint32_t houses = both.toInt32_3(); houses; houses &= (houses - 1)) {
					bm128 tmp = any;
					tmp &= constraints::bitsForHouse[bm128::FindLSBIndex32(houses)]; //mask other candidates and leave only these from the current house
					if(2 == tmp.popcount_128()) {
						//only 2 digits in 2 cells in the same house
						ss |= tmp;
						//ss = tmp;
						pairFound = true;
						//trialCandidates |= ss;
						//break;
					}
				}
				//eliminate the candidates from other digits
				if(pairFound) {
					for(int d = 0; d < 9; d++) {
						if((d != d1) && (d != d2)) {
							if(!grid[d].isDisjoint(ss)) {
								grid[d].clearBits(ss);
								eliminationFound = true;
								//goto nakedAgain;
							}
						}
					}
				}
//				else {
//					knownNoSubsets[subsetIndex] = any;
//				}
			}
		}
		subsetsDone = 1;
		//subsetsDone++;
		if(eliminationFound) {
#ifdef USE_LOCKED_CANDIDATES
			lockedDone = 0;
#endif
			goto nakedAgain;
		}
		//subsetsDone = 1;
	}
#endif //USE_SUBSETS

	//Prepare a guess
	{
		//At this point the existence of unsolved house(s) w/o candidates crashes the algorithm!!!
		bm128* gg = &contexts[guessDepth++][0];

		//Find an unsolved cell with less possibilities
		int optDigit;
		int optCell;

		//find first of the best-to-guess candidates
		{
			bm128 all;
			//try reusing previously found candidates
			all = trialCandidates;
			all.clearBits(solved);
			if(all.isZero()) {
				//find new candidates
				if(2 >= findLeastPopulatedCells(all))
					trialCandidates = all; //reuse them on next guess only if they are bi-values
			}
			for (optDigit = 0; optDigit < 7; optDigit++) {
				if (!all.isDisjoint(grid[optDigit])) {
					all &= grid[optDigit];
					optCell = all.getFirstBit1Index96();
					goto bestCellFound;
				}
			}
			optDigit = 7;
			all &= grid[7];
			optCell = all.getFirstBit1Index96();
		bestCellFound:
			;
		}


#ifdef COUNT_TRIALS
		nTrials++;
#endif

		{
			//bm128* gg = &contexts[guessDepth++][0];
//			__builtin_prefetch(&gg[4]);
//			__builtin_prefetch(&gg[8]);
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
			gg[10] = trialCandidates;
			//later continue with this candidate eliminated
			gg[optDigit].clearBit(optCell);
			//try the "optimal" cell/digit candidate
			collector.setCellValue(optCell, optDigit + 1);
			solved.setBit(optCell); //mark cell as "solved"
			grid[optDigit].clearBits(constraints::visibleCells[optCell]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
			//if(guessDepth == 1) subsetsDone = 0;
			//if(guessDepth == 1) lockedDone = 0;
			goto nakedAgain; //apply direct eliminations
		}
	}

backtrack:
	if(mode & MODE_STOP_GUESSING) { //no need to restore context
		return;
	}
contradiction:
	if(guessDepth-- == 0) { //nothing to restore
		return;
	}
	{
		//We are done with the guess.
		//The caller is notified for each of the the possible solutions found so far
		//Now restore the context. The just guessed candidate has been removed from the context earlier.
		bm128* gg = &contexts[guessDepth][0];
//		__builtin_prefetch(&gg[4]);
//		__builtin_prefetch(&gg[8]);
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
		trialCandidates = gg[10];
		mode = 0;
	}
	goto nakedAgain;
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

//void fsss2::findBiPositionDigit(int& digit, int& cell) const {
//	//find house with less candidates from a particular digit, exit on first bi-position house/digit
//	int minCells = 100;
//	bm128 bestHouse;
//	for(int d = 0; d < 9; d++) {
//		//for each unsolved house
//		//for(uint32_t houses = /*((1 << 27) - 1) &*/(grid[d].toInt64_1()) >> (81 - 64); houses; houses &= (houses - 1)) {
//		for(uint32_t houses = /*((1 << 27) - 1) &*/ grid[d].toInt32_3(); houses; houses &= (houses - 1)) {
//			bm128 tmp = grid[d];
//			tmp &= bitsForHouse[bm128::FindLSBIndex32(houses)];
//			int n = tmp.popcount_128();
//			if(n < minCells) {
//				digit = d;
//				if(n == 2) {
//					cell = tmp.getFirstBit1Index96();
//					return; //not so bad, a bi-position is found
//				}
//				bestHouse = tmp;
//				minCells = n;
//			}
//		}
//		cell = bestHouse.getFirstBit1Index96();
//	}
//	//for(int i = 0; i < 81; i++)
//	//	printf("%c", puzzle[i] + '0');
//	//printf("\t%d\n", minCells);
//}

template < class X > void fsss2 < X > ::solve(const char* const in) {
	//start from clean solver context
	initEmpty();
	//perform optimized setup with the initial givens
	for(int c = 0; c < 81; c++) {
		//int d = in[80 - c]; //<<<<<<<<<<<< reverse the cells
		int d = in[c];
		if(d == 0) {
			//skip non-givens
			continue;
		}
		//d = 10 - d; // <<<<<<<<<<<< reverse the digit
		if(!grid[--d].isBitSet(c)) {
			//direct contradiction within the initial givens
			//mode = MODE_STOP_PROCESSING;
			return;
		}
		collector.setCellValue(c, d + 1);
		solved.setBit(c); //mark cell as "solved"
		grid[d].clearBits(constraints::visibleCells[c]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
	}
	//clear all givens from the candidates in one pass
	//clearSolved(); //unnecessary just before hunting for naked singles
	if(((bm128)constraints::mask81).isSubsetOf(solved)) {
		//all givens :)
		collector.solutionFound();
		return;
	}
	//now do the entire solving process
	doEliminations();
	return;
}

template < class X > void fsss2 < X > ::solve(const uint16_t* const in) {
	//start from clean solver context
	initEmpty();

//	//perform natural setup with the initial givens
//	for(int c = 0; c < 81; c++) {
//		for(int d = 0; d < 9; d++) {
//			if(0 == (1 & (in[c] >> d))) grid[d].clearBit(c);
//		}
//	}

	//perform optimized setup with the initial givens
	bm128 x;
	uint32_t y;
	for(int i = 0; i < 10; i++) { //for each set of 8 cells
		x = &in[i * 8]; //load bitmaps for 8 cells at once
		y = x.mask8(); //bits 0,2,4,6,8,10,12,14 contain pencilmarks for value 8 at cells 0,1,2,3,4,5,6,7
		x.shl16(1);
		y |= (x.mask8() << 16); //plus bits for value 7

		//now de-interleave the 32 bits
		y = (y | (y >> 1)) & 0x33333333; //http://stackoverflow.com/questions/3137266/how-to-de-interleave-bits-unmortonizing
		y = (y | (y >> 2)) & 0x0f0f0f0f;
		y = (y | (y >> 4)) & 0x00ff00ff;
		y = (y | (y >> 8)) & 0x0000ffff;
		grid[7].bitmap128.m128i_u8[i] = y;
		grid[6].bitmap128.m128i_u8[i] = y >> 8;

		x.shl16(1);
		y = x.mask8(); //bits for value 6
		x.shl16(1);
		y |= (x.mask8() << 16); //plus bits for value 5
		y = (y | (y >> 1)) & 0x33333333;
		y = (y | (y >> 2)) & 0x0f0f0f0f;
		y = (y | (y >> 4)) & 0x00ff00ff;
		y = (y | (y >> 8)) & 0x0000ffff;
		grid[5].bitmap128.m128i_u8[i] = y;
		grid[4].bitmap128.m128i_u8[i] = y >> 8;

		x.shl16(1);
		y = x.mask8(); //bits for value 4
		x.shl16(1);
		y |= (x.mask8() << 16); //plus bits for value 3
		y = (y | (y >> 1)) & 0x33333333;
		y = (y | (y >> 2)) & 0x0f0f0f0f;
		y = (y | (y >> 4)) & 0x00ff00ff;
		y = (y | (y >> 8)) & 0x0000ffff;
		grid[3].bitmap128.m128i_u8[i] = y;
		grid[2].bitmap128.m128i_u8[i] = y >> 8;

		x.shl16(1);
		y = x.mask8(); //bits for value 2
		y = (y | (y >> 1)) & 0x33333333;
		y = (y | (y >> 2)) & 0x0f0f0f0f;
		y = (y | (y >> 4)) & 0x00ff00ff;
		//y = (y | (y >> 8)) & 0x0000ffff;
		grid[1].bitmap128.m128i_u8[i] = y;

		x.shl16(1);
		y = x.mask8(); //bits for values 1 and 9
		y |= (y << 15); //move the bits from the odd positions (1 .. 15) to even positions (16 .. 30)
		y &= 0x55555555; //clear the even bits
		y = (y | (y >> 1)) & 0x33333333;
		y = (y | (y >> 2)) & 0x0f0f0f0f;
		y = (y | (y >> 4)) & 0x00ff00ff;
		y = (y | (y >> 8)) & 0x0000ffff;
		grid[0].bitmap128.m128i_u8[i] = y;
		grid[8].bitmap128.m128i_u8[i] = y >> 8;
	}
	//now the last cell
	y = in[80];
	if(y != 511) {
		grid[0].bitmap128.m128i_u8[10] = (y >> 0) & 1;
		grid[1].bitmap128.m128i_u8[10] = (y >> 1) & 1;
		grid[2].bitmap128.m128i_u8[10] = (y >> 2) & 1;
		grid[3].bitmap128.m128i_u8[10] = (y >> 3) & 1;
		grid[4].bitmap128.m128i_u8[10] = (y >> 4) & 1;
		grid[5].bitmap128.m128i_u8[10] = (y >> 5) & 1;
		grid[6].bitmap128.m128i_u8[10] = (y >> 6) & 1;
		grid[7].bitmap128.m128i_u8[10] = (y >> 7) & 1;
		grid[8].bitmap128.m128i_u8[10] = (y >> 8) & 1;
	}

//	//debug
//	char p[88];
//	p[81] = 0;
//	for(int i = 0; i < 9; i++) {
//		grid[i].toMask81(p);
//		printf("%s\n", p);
//	}

	//now do the entire solving process
	doEliminations();
	return;
}

//template class fsss2<getSingleSolution>;
//template class fsss2<hasSingleSolution>;

bool nullCollector::solutionFound() {
	return false;
}
void nullCollector::setCellValue(int cell, int val) {}

//hasSingleSolution::hasSingleSolution() :
//	solver(fsss2<hasSingleSolution>(*this))
//{}
int hasSingleSolution::solve(const char* p) {
	nsol = 0;
	fsss2<hasSingleSolution> solver(*this);
	solver.solve(p);
	return nsol;
}
//int hasSingleSolution::solve(const bm128* p) {
//	nsol = 0;
//	solver.initEmpty();
//	for(int g = 0; g < 9; g++) {
//		solver.grid[g].clearBits(p[g]);
//	}
//	solver.doEliminations();
//	return nsol;
//}
bool hasSingleSolution::solutionFound() {
	return (++nsol == 2);
}

//hasAnySolution::hasAnySolution() :
//	solver(fsss2<hasAnySolution>(*this))
//	{}
int hasAnySolution::solve(const bm128* p) {
	fsss2<hasAnySolution> solver(*this);
	nsol = 0;
	solver.initEmpty();
	for(int g = 0; g < 9; g++) {
		solver.grid[g].clearBits(p[g]);
	}
	solver.doEliminations();
	return nsol;
}
int hasAnySolution::solve(const char* p) {
	fsss2<hasAnySolution> solver(*this);
	nsol = 0;
	solver.solve(p);
	return nsol;
}
bool hasAnySolution::solutionFound() {
	nsol = 1;
	return true;
}

//void isRedundant::setCellValue(int cell, int val) { //debug
//	if(cell < 0 || cell > 80 || val < 1 || val > 9)
//		return;
//	sol[cell] = val;
//}

bool isRedundant::solutionFound() {
	nsol = 1;
	return true;
}
bool isRedundant::solve(const char* p, int c) {
	fsss2<isRedundant> solver(*this);
	nsol = 0;
	solver.initEmpty();
	for(int i = 0; i < 81; i++) {
		if(i != c) {
			int g = p[i];
			if(g) {
				solver.setCellValue(i, g);
			}
		}
		else {
			solver.eliminateCellValue(i, p[i]);
		}
	}
	solver.doEliminations();
	return (nsol == 0);
}
bool isRedundant::solve(const bm128* p, int given, int position) {
	fsss2<isRedundant> solver(*this);
	nsol = 0;
	solver.initEmpty();
	bm128 posMask = bitSet[position];
	for(int g = 0; g < 9; g++) {
		solver.grid[g].clearBits(p[g]);
		if(g != given) {
			solver.grid[g].clearBits(posMask);
		}
		else {
			solver.grid[g] |= posMask;
		}
	}
	solver.doEliminations();
	return (nsol != 0);
}

bool isIrreducible::solve(const char* p) {
	fsss2<isIrreducible> solver(*this);
	int pos[81], nGivens = 0;
	uint16_t valBM[81];

//	for(int c = 0; c < 81; c++) {
//		valBM[c] = 0;
//	}
//	for(int testCell = 0; testCell < 81; testCell++) {
//		for(int testBit = 0; testBit < 9; testBit++) {
//			valBM[testCell] = 1 << testBit;
//			printf("\n%d,%d\n", testCell, testBit);
//			solver.solve(valBM);
//			valBM[testCell] = 0;
//		}
//	}
//	for(int c = 0; c < 81; c++) {
//		valBM[c] = 511;
//	}
//	for(int testCell = 0; testCell < 81; testCell++) {
//		for(int testBit = 0; testBit < 9; testBit++) {
//			valBM[testCell] = 511 & ~(1 << testBit);
//			printf("\n%d,%d\n", testCell, testBit);
//			solver.solve(valBM);
//			valBM[testCell] = 511;
//		}
//	}

	int dc[9] = {0,0,0,0,0,0,0,0,0};
	for(int c = 0; c < 81; c++) {
		if(p[c] == 0) {
			valBM[c] = 511;
			continue;
		}
		pos[nGivens++] = c;
		valBM[c] = 1 << (p[c] - 1);
		if(++dc[p[c] - 1] > 6)
			return false; //this works for 36+ givens
	}
	for(int skip = 0; skip < nGivens; skip++) {
		nsol = 0;
		//for this cell check whether there exist a solution having different value than the given
		valBM[pos[skip]] ^= 511;
		solver.solve(valBM);
		if(nsol == 0)
			return false; //all solutions with the rest of the givens fixed require this cell to have the same value as the given => this given is redundant
		valBM[pos[skip]] ^= 511;
	}
	//all tests passed
	return true;
}
bool isIrreducible::solutionFound() {
	nsol = 1;
	return true;
}

void getSingleSolution::setCellValue(int cell, int val) {
	if(nsol) return; //store only the first solution
	resChar[cell] = val;
}
bool getSingleSolution::solutionFound() {
	return (++nsol == 2); //stop after possible second solution
}
int getSingleSolution::solve(const char* p, char* res) {
	fsss2<getSingleSolution> solver(*this);
	resChar = res;
	solver.solve(p);
	return nsol;
}
int getSingleSolution::solve(const bm128* p, char* res) {
	fsss2<getSingleSolution> solver(*this);
	nsol = 0;
	resChar = res;
	solver.initEmpty();
	for(int g = 0; g < 9; g++) {
		solver.grid[g].clearBits(p[g]);
	}
	solver.doEliminations();
	return nsol;
}

