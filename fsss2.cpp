/*
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

#ifdef COUNT_TRIALS
extern int nTrials;
#endif

//int knownNoLockedCandidatesHits;
//int knownNoLockedCandidatesMisses;
//int knownNoHiddenHits;
//int knownNoHiddenMisses;

template <class X> fsss2<X>::fsss2(X &theCollector) : mode(0), guessDepth(0)
#ifdef USE_LOCKED_CANDIDATES
#ifndef LOCKED_CANDIDATES_ALWAYS
		, lockedDone(0)
#endif
#endif
, collector(theCollector)
{}

//only first 81 bits set
constexpr t_128 constraints::mask81 = {0xFFFFFFFFFFFFFFFF,    0x0001FFFF};

//only first 81 + 27 = 108 bits set
//template <class X> const t_128 fsss2<X>::mask108 = {0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFF};

//the first 81 bits for the cells and the 27 bits from position 96+
constexpr t_128 constraints::mask108 = {0xFFFFFFFFFFFFFFFF,0x07FFFFFF0001FFFF};

//the 27 bits from position 96+
//const t_128 constraints::mask27 = {0x0,0x07FFFFFF00000000};

constexpr t_128 constraints::visibleCells[81] = { //1 for all 20 visible cells, 1 for the cell itself, 1 for the three houses
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

constexpr t_128 constraints::bitsForHouse[27] = { //1 for the 9 cells in the house
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
constexpr tripletMask constraints::tripletMasks[54] = {
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

template <class X> constexpr void fsss2<X>::initEmpty() {
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
#ifdef LOCKED_CANDIDATES_USE_CACHE
	knownNoLockedCandidates[0] = constraints::mask108;
	knownNoLockedCandidates[1] = constraints::mask108;
	knownNoLockedCandidates[2] = constraints::mask108;
	knownNoLockedCandidates[3] = constraints::mask108;
	knownNoLockedCandidates[4] = constraints::mask108;
	knownNoLockedCandidates[5] = constraints::mask108;
	knownNoLockedCandidates[6] = constraints::mask108;
	knownNoLockedCandidates[7] = constraints::mask108;
	knownNoLockedCandidates[8] = constraints::mask108;
#endif
#ifndef LOCKED_CANDIDATES_ALWAYS
	lockedDone = 0;
#endif
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
//template <class X> bool fsss2<X>::doLockedCandidatesForDigit(bm128& tmp) {
//	bool found = false;
//	for(uint32_t lines = 0x03FFFF & (tmp.toInt32_3()); lines; lines &= (lines - 1)) {
//		unsigned int line = 3U * bm128::FindLSBIndex32(lines); //unsolved row or column
//		//process the 3 triplets in the line (row or column)
//		for(unsigned int t = 0; t < 3; t++) {
//			//if(tmp.isDisjoint(constraints::tripletMasks[line + t].self)) continue; //no candidates, possibly resolved box
//			bool dl = tmp.isDisjoint(constraints::tripletMasks[line + t].adjacentLine);
//			bool db = tmp.isDisjoint(constraints::tripletMasks[line + t].adjacentBox);
//			if(dl) {
//				//unsolved line with candidates located only within this triplet
//				if(!db) {
//					tmp.clearBits(constraints::tripletMasks[line + t].adjacentBox);
//					found = true;
//					//goto next_line;
//					return true;
//				}
//			}
//			if(db) {
//				//the other two triplets within this box have no candidates
//				if(!dl) {
//					//other two triplets for this line have candidates, but the box is possibly solved
//					if(!tmp.isDisjoint(constraints::tripletMasks[line + t].self)) { //unsolved box
//						tmp.clearBits(constraints::tripletMasks[line + t].adjacentLine);
//						found = true;
//						//goto next_line;
//						return true;
//					}
//				}
//			}
//		} //triplets in a line
////next_line:
//		;
//	} //lines
//	return found;
//	//return false;
//}
template <class X> bool fsss2<X>::doLockedCandidatesForDigit(bm128& tmp) {
	static constexpr int boxNum[18][3] = {
			{0,1,2},{0,1,2},{0,1,2},{3,4,5},{3,4,5},{3,4,5},{6,7,8},{6,7,8},{6,7,8},
			{0,3,6},{0,3,6},{0,3,6},{1,4,7},{1,4,7},{1,4,7},{2,5,8},{2,5,8},{2,5,8}
	};
	bool found = false;
	uint32_t unsolvedHouses = tmp.toInt32_3();
	if(unsolvedHouses == 0) return false;
	uint32_t unsolvedBoxes = unsolvedHouses >> 18;
	//if(unsolvedBoxes == 0) return false; //never happens
	for(uint32_t lines = 0x03FFFF & unsolvedHouses; lines; lines &= (lines - 1)) {
		unsigned int line = bm128::FindLSBIndex32(lines); //unsolved row or column
		unsigned int lineBase = 3U * line;
		//process the 3 triplets in the line (row or column)
		for(unsigned int t = 0; t < 3; t++) {
			if(0 == (unsolvedBoxes & ((uint32_t) 1) << (boxNum[line][t]))) continue; //resolved box
			//if(tmp.isDisjoint(constraints::tripletMasks[lineBase + t].self)) continue; //resolved box
//				fprintf(stderr, "?");
//				continue; //no candidates, possibly resolved box
//			}
			bool dl = tmp.isDisjoint(constraints::tripletMasks[lineBase + t].adjacentLine); //all line candidates are within this triplet
			bool db = tmp.isDisjoint(constraints::tripletMasks[lineBase + t].adjacentBox); //all box candidates are within this triplet
			if(dl) {
				//unsolved line with candidates located only within this triplet
				if(!db) {
					tmp.clearBits(constraints::tripletMasks[lineBase + t].adjacentBox);
					found = true;
					//goto next_line;
					return true;
				}
			}
			if(db) {
				//the other two triplets within this box have no candidates
				if(!dl) {
					//other two triplets for this line have candidates, but the box is possibly solved
					//if(!tmp.isDisjoint(constraints::tripletMasks[lineBase + t].self)) { //unsolved box
					//if(1 || 0 != (unsolvedBoxes & (((uint32_t) 1) << (boxNum[line][t])))) {
						tmp.clearBits(constraints::tripletMasks[lineBase + t].adjacentLine);
						found = true;
						//goto next_line;
						return true;
					//}
				}
			}
		} //triplets in a line
//next_line:
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

template <class X> void fsss2<X>::doNakedSingles() { //cells with only one remaining candidate
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
			{
				register bm128 tmp;
				tmp = g0; tmp &= all; duplicates |= tmp; all |= g0;
				tmp = g1; tmp &= all; duplicates |= tmp; all |= g1;
				tmp = g2; tmp &= all; duplicates |= tmp; all |= g2;
				tmp = g3; tmp &= all; duplicates |= tmp; all |= g3;
				tmp = g4; tmp &= all; duplicates |= tmp; all |= g4;
				tmp = g5; tmp &= all; duplicates |= tmp; all |= g5;
				tmp = g6; tmp &= all; duplicates |= tmp; all |= g6;
				tmp = g7; tmp &= all; duplicates |= tmp; all |= g7;
				tmp = g8; tmp &= all;                    all |= g8;
				if(((bm128)constraints::mask81).isSubsetOf(all)) {
					;
				}
				else {
					//there is at least one unsolved cell without any candidate
					goto contradiction;
				}
				duplicates |= tmp;
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
	} while(!((bm128)constraints::mask81).isSubsetOf(slv));
	//finally all 81 cells are solved

	//a slower alternative for contradiction checking within the loops
//	slv = g0;
//	slv |= g1;
//	slv |= g2;
//	slv |= g3;
//	slv |= g4;
//	slv |= g5;
//	slv |= g6;
//	slv |= g7;
//	slv |= g8;
//	if(!slv.isZero())
//		goto contradiction;

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
	bool guessAuto = true;
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
					//knownNoHiddenHits++;
					continue;
				}
				//knownNoHiddenMisses++;
againSameHidden:
				//for each unsolved house
				for(uint32_t houses = grid[d].toInt32_3(); houses; houses &= (houses - 1)) {
					uint32_t house = bm128::FindLSBIndex32(houses);
					bm128 tmp = grid[d];
					tmp &= constraints::bitsForHouse[house]; //mask other candidates and leave only these from the current house

					//if(tmp.isZero()) goto contradiction;

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
#ifndef LOCKED_CANDIDATES_ALWAYS
	if(lockedDone == 0)
#endif
	{
		bool found = false;
		//if a digit in a row is within a single triplet, then remove digit from the other two box triplets and vice versa
		for (int d = 0; d < 9; d++) {
#ifdef LOCKED_CANDIDATES_USE_CACHE
			if(knownNoLockedCandidates[d] == grid[d]) {
				//knownNoLockedCandidatesHits++;
				continue;
			}
			//knownNoLockedCandidatesMisses++;
			if(doLockedCandidatesForDigit(grid[d])) {
				found = true;
			}
			else {
				knownNoLockedCandidates[d] = grid[d];
			}
#else
			found |= doLockedCandidatesForDigit(grid[d]);
#endif
		}
#ifndef LOCKED_CANDIDATES_ALWAYS
		lockedDone = 1;
#endif
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
	//if(subsetsDone == 0) {
	if(1 || subsetsDone < 5) {
		bool eliminationFound = false;
		//int subsetIndex = 0; //01,02,03,04,05,06,07,08,12,13,14,15,16,17,18,23,24,25,26,27,28,34,35,36,37,38,45,46,47,48,56,57,58,67,68,78
		for(int d1 = 0, subsetIndex = 0; d1 < 8; d1++) {
			for(int d2 = d1 + 1; d2 < 9; d2++, subsetIndex++) {
			//for(int d2 = d1 + 1; d2 < 9; d2++) {
				bm128 any = grid[d1];
				any |= grid[d2];
				if(any == knownNoSubsets[subsetIndex])
					continue;
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
				else {
					knownNoSubsets[subsetIndex] = any;
				}
			}
		}
		//subsetsDone = 1;
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
nextGuess:
	{
		//At this point the existence of unsolved house(s) w/o candidates crashes the algorithm!!!

		//Find an unsolved cell with less possibilities
		int optDigit;
		int optCell;
		int minCellValues = 2;
		guessAuto = collector.beforeGuess(guessDepth, optCell, optDigit);

		//find first of the best-to-guess candidates
		if(guessAuto) {
			bm128 all;
			//try reusing previously found candidates
			all = trialCandidates;
			all.clearBits(solved);
			if(all.isZero()) {
				//find new candidates
				minCellValues = findLeastPopulatedCells(all);
				if(minCellValues == 2) {
					trialCandidates = all; //reuse them on next guess only if they are bi-values
				}
//				else if(minCellValues > 3) {
//					//find house with less candidates from a particular digit, exit on first bi-position house/digit
//					int minValueLocations = 100;
//					for (int d = 0; d < 9; d++) {
//						for (int h = 0; h < 27; h++) {
//							if (!grid[d].isBitSet(81 + h))
//								continue;
//							bm128 tmp = grid[d];
//							tmp &= constraints::bitsForHouse[h];
//							tmp &= constraints::mask81;
//							int n = tmp.popcount_128();
//							if (n < minValueLocations) {
//								optDigit = d;
//								optCell = tmp.getFirstBit1Index96();
//								if(n == 2) { //not so bad, a bi-position is found
//									//break;
//									goto bestCellFound;
//								}
//								minValueLocations = n;
//							}
//						}
//					}
//					if(minValueLocations < minCellValues) { //prefer n-location
//						goto bestCellFound;
//					}
//				}
			}
#ifdef GUESS_STRATEGY_2
			//strategy 1: least value, then least cell index //234.922 seconds. Trials = 737 680 539
			//if((guessDepth & 1) == 0) //flip-flop
			//if(solved.popcount_128() > 18)
			//if(solved.popcount_128() < 10) //low-clue pencilmark-only puzzles
			if(minCellValues == 2 && solved.popcount_128() > 22)
			//if(minCellValues == 2)
			//if(0)
#endif
			{
				for(optDigit = 0; optDigit < 7; optDigit++) { //prefer the least value
					if(!all.isDisjoint(grid[optDigit])) {
						all &= grid[optDigit];
						optCell = all.getFirstBit1Index96(); //prefer the lefttmost position (unless reversely initialized)
						goto bestCellFound;
					}
				}
				optDigit = 7; //digit 8 must have optCell, worst case not covered above is a bi-value {8,9} cell
				all &= grid[7];
				optCell = all.getFirstBit1Index96();
			}
#ifdef GUESS_STRATEGY_2
			else
			//strategy 2a (experimental): most direct eliminations for the GUESSED value
			{
				bm128 uncheckedCells(all);
				int maxEliminations = 0;
				for(uint64_t cells = uncheckedCells.toInt64(); cells; cells &= (cells - 1)) {
					uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
					for(int d = 0; d < 9; d++) {
						if(!grid[d].isBitSet(cell)) continue;
						bm128 eliminations(grid[d]);
						eliminations &= constraints::visibleCells[cell];
						eliminations &= constraints::mask81;
						int numEliminations = eliminations.popcount_128();
						if(maxEliminations < numEliminations) {
							optDigit = d;
							optCell = cell;
							if(numEliminations >= 20) goto bestCellFound;
							maxEliminations = numEliminations;
						}
					}
				} //for lower 64 cells
				for(uint32_t cells = uncheckedCells.toInt32_2(); cells; cells &= (cells - 1)) {
					uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
					for(int d = 0; d < 9; d++) {
						if(!grid[d].isBitSet(cell)) continue;
						bm128 eliminations(grid[d]);
						eliminations &= constraints::visibleCells[cell];
						eliminations &= constraints::mask81;
						int numEliminations = eliminations.popcount_128();
						if(maxEliminations < numEliminations) {
							optDigit = d;
							optCell = cell;
							if(numEliminations >= 20) goto bestCellFound;
							maxEliminations = numEliminations;
						}
					}
				}
			}
//			{ //strategy 3: most direct eliminations for all values;  slower than 2a alternative
//				bm128 uncheckedCells(all);
//				int maxEliminations = 0;
//				for(uint64_t cells = uncheckedCells.toInt64(); cells; cells &= (cells - 1)) {
//					uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
//					bm128 eliminations(constraints::visibleCells[cell]);
//					eliminations.clearBits(solved);
//					int numEliminations = eliminations.popcount_128();
//					if(maxEliminations < numEliminations) {
//						optCell = cell;
//						maxEliminations = numEliminations;
//					}
//				} //for lower 64 cells
//				for(uint32_t cells = uncheckedCells.toInt32_2(); cells; cells &= (cells - 1)) {
//					uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
//					bm128 eliminations(constraints::visibleCells[cell]);
//					eliminations.clearBits(solved);
//					int numEliminations = eliminations.popcount_128();
//					if(maxEliminations < numEliminations) {
//						optCell = cell;
//						maxEliminations = numEliminations;
//					}
//				}
//				for(int d = 0; d < 9; d++) {
//					if(!grid[d].isBitSet(optCell)) continue;
//					optDigit = d;
//					break;
//				}
//			}
//			{ //strategy 4: guess within the house with least solved cells
//				//determine best house
//				int minSolvedInHouse = 100;
//				int bestHouse = 0;
//				for(int h = 0; h < 27; h++) {
//					if(all.isDisjoint(constraints::bitsForHouse[h])) continue;
//					bm128 solvedInHouse(solved);
//					solvedInHouse &= constraints::bitsForHouse[h];
//					int numSolvedInHouse = solvedInHouse.popcount_128();
//					if(minSolvedInHouse > numSolvedInHouse) {
//						bestHouse = h;
//						if(numSolvedInHouse == 0) break;
//						minSolvedInHouse = numSolvedInHouse;
//					}
//				}
//				//choose a candidate from the best house
//				bm128 candidateCells(all);
//				candidateCells &= constraints::bitsForHouse[bestHouse];
//				optCell = candidateCells.getFirstBit1Index96();
//				for(int d = 0; d < 9; d++) {
//					if(!grid[d].isBitSet(optCell)) continue;
//					optDigit = d;
//					break;
//				}
//			}
#endif
//			//strategy 3 (experimental, for completeness): least direct eliminations value/cell //439.740 seconds. Trials = 1 219 276 128
//			{
//				bm128 uncheckedCells(all);
//				int minEliminations = 100;
//				for(uint64_t cells = uncheckedCells.toInt64(); cells; cells &= (cells - 1)) {
//					uint32_t cell = (uint32_t) bm128::FindLSBIndex64(cells); //get the rightmost bit index
//					for(int d = 0; d < 9; d++) {
//						if(!grid[d].isBitSet(cell)) continue;
//						bm128 eliminations(grid[d]);
//						eliminations &= constraints::visibleCells[cell];
//						int numEliminations = eliminations.popcount_128();
//						if(minEliminations > numEliminations) {
//							optDigit = d;
//							optCell = cell;
//							minEliminations = numEliminations;
//						}
//					}
//				} //for lower 64 cells
//				for(uint32_t cells = uncheckedCells.toInt32_2(); cells; cells &= (cells - 1)) {
//					uint32_t cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
//					for(int d = 0; d < 9; d++) {
//						if(!grid[d].isBitSet(cell)) continue;
//						bm128 eliminations(grid[d]);
//						eliminations &= constraints::visibleCells[cell];
//						int numEliminations = eliminations.popcount_128();
//						if(minEliminations > numEliminations) {
//							optDigit = d;
//							optCell = cell;
//							minEliminations = numEliminations;
//						}
//					}
//				}
//			}
		bestCellFound:
			;
		}
		else {
			if(mode) {
				goto backtrack;
			}
		}


#ifdef COUNT_TRIALS
		nTrials++;
#endif

		{
			bm128* gg = &contexts[guessDepth++][0];
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
			if(guessAuto)gg[optDigit].clearBit(optCell); //unless the guess choice has been enforced by the collector
		}
		//try the "optimal" cell/digit candidate
		collector.setCellValue(optCell, optDigit + 1);
		solved.setBit(optCell); //mark cell as "solved"
		grid[optDigit].clearBits(constraints::visibleCells[optCell]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
		//if(guessDepth == 1) subsetsDone = 0;
		//if(guessDepth == 1) lockedDone = 0;
		goto nakedAgain; //apply direct eliminations
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
		//fprintf(stderr, "restoring to guessDepth=%d\n", guessDepth); //debug
		//We are done with the guess.
		//The caller is notified for each of the the possible solutions found so far
		//Now restore the context. The just guessed candidate has been removed from the context earlier (unless it was an enforced guess).
		bm128* gg = &contexts[guessDepth][0];
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
	if(guessAuto) goto nakedAgain;
	goto nextGuess;
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
//		collector.setCellValue(c, d + 1);
//		solved.setBit(c); //mark cell as "solved"
//		grid[d].clearBits(constraints::visibleCells[c]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
		setCellValue(c, d + 1);
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

template < class X > void fsss2 < X > ::solve(const pencilmarks& forbidden) {
	initEmpty();
	for(int g = 0; g < 9; g++) {
		grid[g].clearBits(forbidden[g]);
	}
	doEliminations();
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

bool nullCollector::solutionFound() {return false;} //continue
void nullCollector::setCellValue(int cell, int val) {}
bool nullCollector::beforeGuess(int guessDepth, int &optCell, int &optDigit) {return true;} //guess Auto

int hasSingleSolution::solve(const char* p) {
	nsol = 0;
	fsss2<hasSingleSolution> solver(*this);
	solver.solve(p);
	return nsol;
}
int hasSingleSolution::solve(const pencilmarks& p) {
	nsol = 0;
	fsss2<hasSingleSolution> solver(*this);
	solver.solve(p);
	return nsol;
}
bool hasSingleSolution::solutionFound() {
	return (++nsol == 2);
}

int hasAnySolution::solve(const pencilmarks& p) {
	fsss2<hasAnySolution> solver(*this);
	nsol = 0;
	solver.solve(p);
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

int noGuess::solve(const pencilmarks& p) {
	fsss2<noGuess> solverInstance(*this);
	solverPtr = &solverInstance;
	nsol = 0;
	solverInstance.solve(p);
	return nsol;
}
int noGuess::reduce(pencilmarks& p) {
	fsss2<noGuess> solverInstance(*this);
	solverPtr = &solverInstance;
	nsol = 0;
	solverInstance.solve(p);
	p.fromSolver(solverInstance.grid);
	return nsol;
}
int noGuess::solve(const char* p) {
	fsss2<noGuess> solverInstance(*this);
	solverPtr = &solverInstance;
	nsol = 0;
	solverInstance.solve(p);
	return nsol;
}
bool noGuess::solutionFound() {
	nsol = 1;
	return true;
}
bool noGuess::beforeGuess(int guessDepth, int &optCell, int &optDigit) {
	solverPtr->mode = MODE_STOP_PROCESSING | MODE_STOP_GUESSING; //exit (actually MODE_STOP_GUESSING is redundant here)
	return false;
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
bool isRedundant::solve(const char* p, int testPosition) {
	fsss2<isRedundant> solver(*this);
	nsol = 0;
	solver.initEmpty();
	for(int i = 0; i < 81; i++) {
		if(i != testPosition) {
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
bool isRedundant::solve(const pencilmarks& forbiddenValuePositions, int testValue, int testPosition) {
	fsss2<isRedundant> solver(*this);
	nsol = 0;
	solver.initEmpty(); //all values are allowed everywhere
	bm128 posMask = bitSet[testPosition];
	for(int g = 0; g < 9; g++) {
		solver.grid[g].clearBits(forbiddenValuePositions[g]); //forbidden values are disallowed
		if(g != testValue) {
			solver.grid[g].clearBits(posMask); //disallow all testValue alternatives in this cell
		}
		else {
			solver.grid[g] |= posMask; //allow only testValue in this cell
		}
	}
	solver.doEliminations();
	return (nsol == 0); //if no solution exist, then forbidding the testValue is redundant
}

bool isIrreducible::solve(const char* p) {
	fsss2<isIrreducible> solver(*this);
	int pos[81], nGivens = 0;
	uint16_t valBM[81];

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
	nsol = 0;
	resChar = res;
	solver.solve(p);
	return nsol;
}
int getSingleSolution::solve(const pencilmarks& forbiddenValuePositions, char* res) {
	fsss2<getSingleSolution> solver(*this);
	nsol = 0;
	resChar = res;
	solver.solve(forbiddenValuePositions);
	return nsol;
}

bool singleSolutionGuesses::beforeGuess(int guessDepth, int &optCell, int &optDigit) {
	numGuesses[guessDepth]++;
	return true;
}
void singleSolutionGuesses::setCellValue(int cell, int val) {
	if(nsol) return; //store only the first solution
	resChar[cell] = val;
}
bool singleSolutionGuesses::solutionFound() {
	return (++nsol == 2); //stop after possible second solution
}
int singleSolutionGuesses::solve(const char* p, char* res, int* numGuessesByDepth) {
	fsss2<singleSolutionGuesses> solver(*this);
	nsol = 0;
	resChar = res;
	numGuesses = numGuessesByDepth;
	for(int i = 0; i < 81; i++) {
		numGuesses[i] = 0;
	}
	solver.solve(p);
	return nsol;
}
int singleSolutionGuesses::solve(const pencilmarks& forbiddenValuePositions, char* res, int* numGuessesByDepth) {
	fsss2<singleSolutionGuesses> solver(*this);
	nsol = 0;
	resChar = res;
	numGuesses = numGuessesByDepth;
	for(int i = 0; i < 81; i++) {
		numGuesses[i] = 0;
	}
	solver.solve(forbiddenValuePositions);
	return nsol;
}

void getTwoSolutions::setCellValue(int cell, int val) {
	resChar[cell] = val;
}
bool getTwoSolutions::solutionFound() {
	if(nsol == 0) {
		for(int i = 0; i < 81; i++) {
			resChar[i + 81] = resChar[i];
		}
		resChar += 81;
		nsol++;
		return false;
	}
	nsol++;
	return true; //stop after possible second solution
}
int getTwoSolutions::solve(const char* p, char* res) {
	fsss2<getTwoSolutions> solver(*this);
	nsol = 0;
	resChar = res;
	solver.solve(p);
	return nsol;
}
int getTwoSolutions::solve(const pencilmarks& forbiddenValuePositions, char* res) {
	fsss2<getTwoSolutions> solver(*this);
	nsol = 0;
	resChar = res;
	solver.solve(forbiddenValuePositions);
	return nsol;
}

bool getAnySolution::solutionFound() {
	return (++nsol == 1); //stop after first solution
}
int getAnySolution::solve(const char* p, char* res) {
	fsss2<getAnySolution> solver(*this);
	nsol = 0;
	resChar = res;
	solver.solve(p);
	return nsol;
}
int getAnySolution::solve(const pencilmarks& forbiddenValuePositions, char* res) {
	fsss2<getAnySolution> solver(*this);
	nsol = 0;
	resChar = res;
	solver.solve(forbiddenValuePositions);
	return nsol;
}

void multiSolutionPM::setCellValue(int cell, int val) {
	sol[cell] = val;
}
bool multiSolutionPM::solutionFound() {
	for(int c = 0; c < 81; c++) {
		resPM[sol[c] - 1].setBit(c);
	}
	++nsol;
	return nsol == solutionsLimit;
}
int multiSolutionPM::solve(const char* p, pencilmarks& res, int maxSolutions) {
	fsss2<multiSolutionPM> solver(*this);
	nsol = 0;
	resPM = res;
	solutionsLimit = maxSolutions;
	resPM.clear();
	solver.solve(p);
	return nsol == solutionsLimit ? 0 : nsol;
}
int multiSolutionPM::solve(const pencilmarks& forbiddenValuePositions, pencilmarks& res, int maxSolutions) {
	fsss2<multiSolutionPM> solver(*this);
	nsol = 0;
	resPM = res;
	solutionsLimit = maxSolutions;
	resPM.clear();
	solver.solve(forbiddenValuePositions);
	return nsol == solutionsLimit ? 0 : nsol;
}
int multiSolutionPM::solve(const pencilmarks& forbiddenValuePositions, pencilmarks& res) {
	isRedundant rt;
	res.clear();
	int ret = 0;
	for(int d = 0; d < 9; d++) {
		for(int c = 0; c < 81; c++) {
			if(forbiddenValuePositions[d].isBitSet(c)) continue; //test only allowed
			if(rt.solve(forbiddenValuePositions, d, c)) {
				res[d].setBit(c);
				ret++;
			}
		}
	}
	return ret;
}

patEnum::patEnum() :
	solver(fsss2<patEnum>(*this)), nsol(0), size(0), numFixedValues(0), curGuessDepth(0), numPuzzles(0)
{}
//void patEnum::setCellValue(int cell, int val) {
//	if(nsol) return; //store only the first solution
//	resChar[cell] = val;
//}
bool patEnum::solutionFound() {
	//at this point in pp[] we have a puzzle candidate with at least one solution
	if(nsol++) { //multiple solutions
		if((int)solver.guessDepth > size) // beyond the enforced placements?
			solver.guessDepth = size; //backtrack to the last enforced cell, guessDepth will be decremented by the solver
	}
	return false; //stop from beforeGuess() handler, not from here
}
int patEnum::solve(const char* p, const bm128* fixed) {
	nsol = 0;
	init(p, fixed);
	solver.solve(pp);
	return 0;
}
bool patEnum::beforeGuess(int guessDepth, int &optCell, int &optDigit) {
	//possible outputs:
	// - auto = set optCell to optDigit, eliminate it from further processing, store context, increment guessDepth and do eliminations (return true)
	// - exit = return (set solver.mode to MODE_STOP_PROCESSING, return false)
	// - backtrack to X and continue = do eliminations (restore context, set optCell and optDigit, set solver.guessDepth, leave solver.mode unset, return false)

	if(guessDepth >= size) return true; //all cells of interest are set

#ifdef USE_LOCKED_CANDIDATES
	solver.lockedDone = 0;
#endif //USE_LOCKED_CANDIDATES

#ifdef USE_SUBSETS
	solver.subsetsDone = 0;
#endif

	//if(guessDepth == size - 9) fprintf(stderr, "*"); //debug

	//if we are going deeply, then choose the cell and obtain the values to iterate
	if(guessDepth > curGuessDepth) {
		//choose cell
		if(!unsetCells[guessDepth].isDisjoint(solver.solved)) {
			//one of the unset givens is already solved, puzzle is not minimal
			//backtrack to previous guessDepth
			//fprintf(stderr, "m"); //debug
			solver.mode = MODE_STOP_PROCESSING;
			return false;
		}
		bm128 best;
		{
			bm128 oldSolved = solver.solved;
			solver.solved = constraints::mask81;
			solver.solved.clearBits(unsetCells[guessDepth]);
			solver.findLeastPopulatedCells(best);
			solver.solved = oldSolved;
		}
		//if(best.isZero()) fprintf(stderr, "U\n"); //debug
		optCell = best.getFirstBit1Index96();
		bm128 cellBM = bitSet[optCell];
		cellCandidates[guessDepth] = 0;
		//int maxVal = (numFixedValues + guessDepth) > 8 ? 9 : numFixedValues + guessDepth + 1; //allow all used and first of the unused values
		//for(int d = 0; d < maxVal; d++) {
		for(int d = 0; d < 9; d++) {
			if(solver.grid[d].isDisjoint(cellBM)) continue; //skip eliminated candidates
			cellCandidates[guessDepth] |= (1 << d); //add this candidate
		}
		//mask all unused values but one,
		//i.e. if values 1..4 have been used so far, then add only 5 as candidate, but not 6..9 since they result in puzzles that are isomorphic to the one with 5 set
		//how? Take the less significant bit from the unused values and mask candidates with (used values | lsb from the unused values)
		if(usedValues[guessDepth] == 511) {
			;
		}
		else {
			uint32_t unusedValues = ((uint32_t)511) ^ usedValues[guessDepth];
			cellCandidates[guessDepth] &= (usedValues[guessDepth] | (((uint32_t)1) << bm128::FindLSBIndex32(unusedValues)));
		}

		//we know a puzzle must have givens with at least 8 different digits. Check this.
		//on size=20 and guessDepth=18 we must have 7 digits already set
		int32_t numDifferentValues = _popcnt32(usedValues[guessDepth]);
		if(numDifferentValues < 8) {
			//if(size - guessDepth - 1 == 8 - numDifferentValues) { //maximum different values from further givens == minimum new values to be used
			if(size - guessDepth == 8 - numDifferentValues) { //maximum different values from this and any further givens == minimum new values that must be used
				//we are enforced to use only new digits. Clear the used ones from the candidates.
				cellCandidates[guessDepth] &= ~usedValues[guessDepth];
			}
		}

		chosenGuessCell[guessDepth] = optCell;
		//remove the chosen cell from the further guessing candidates
		unsetCells[guessDepth + 1] = unsetCells[guessDepth];
		unsetCells[guessDepth + 1].clearBits(cellBM);
	}

	curGuessDepth = guessDepth;

	//in the latest controlled guess check whether a valid puzzle is found in the previous iteration
	if(guessDepth == (size - 1)) {
		if(nsol == 1) {
			//a single solution puzzle was found in previous iteration and is placed in pp[]
			isIrreducible ss;
			if(ss.solve(pp)) {
				//lucky!
				char ppp[88];
				for(int i = 0; i < 81; i++) {
					ppp[i] = pp[i] ? pp[i] + '0' : '.';
				}
				fprintf(stdout, "%81.81s\n", ppp);
				fflush(NULL);
				numPuzzles++;
				if(numPuzzles > 1000000000) exit(0); //for profile generation
			}
		}
	}

	//solutionFound() handler can bring us here when an "incomplete" puzzle is solved w/o guessing, giving nsol > 0
	//since we are about to move to the next candidate, clearing the nsol is necessary in order not to mix solutions from different iterations
	nsol = 0;

	while(curGuessDepth >= 0 && cellCandidates[curGuessDepth] == 0) {
		curGuessDepth--;
	}
	if(curGuessDepth != guessDepth) {
		if(curGuessDepth < 0) {
			//we are done
			solver.guessDepth = 0; //enforce exit
			solver.mode = MODE_STOP_PROCESSING | MODE_STOP_GUESSING; //exit (actually MODE_STOP_GUESSING is redundant here)
			return false;
		}
		solver.guessDepth = curGuessDepth + 1;
		solver.mode = MODE_STOP_PROCESSING; //load context and return back
		return false;
	}
	//choose first unprocessed candidate for this cell
	optCell = chosenGuessCell[curGuessDepth];
	optDigit = bm128::FindLSBIndex32(cellCandidates[curGuessDepth]);
	cellCandidates[curGuessDepth] ^= (1 << optDigit); //exclude this value from later iterations
	pp[optCell] = optDigit + 1; //save it for later checking and output
	usedValues[curGuessDepth + 1] = usedValues[curGuessDepth] | (1 << optDigit);
	//fprintf(stderr, "(%d:%d=%d%s)\n", curGuessDepth, optCell, pp[optCell], cellCandidates[curGuessDepth] ? "" : ", last"); //debug
	return false; //enforce the solver to guess this cell/digit
}
void patEnum::init(const char *puz, const bm128* fixed) {
	size = 0;
	curGuessDepth = -1;
	numFixedValues = 0;
	numPuzzles = 0;

	//set the rest of the class members
	usedValues[0] = 0; //the values already in use
	unsetCells[0].clear();
	for(int i = 0; i < 81; i++) {
		pp[i] = 0;
		if(puz[i] == 0) continue;
		if(fixed != NULL && fixed->isBitSet(i)) {
			pp[i] = puz[i];
			++numFixedValues;
			usedValues[0] |= (1 << pp[i]);
		}
		else {
			unsetCells[0].setBit(i);
			size++;
		}
	}
	//at this stage we have
	// - number of cells with fixed position but floating value, in size
	// - number of fixed values at the top of the positions, in numFixedValues. Unused.
	// - a subgrid with populated fixed values and cleared others, in pp[]
}
