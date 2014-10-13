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

//only first 81 bits set
const t_128 fsss2::mask81 = {0xFFFFFFFFFFFFFFFF,0x0001FFFF};

const t_128 fsss2::minus1 = {0xffffffffffffffff,0xffffffffffffffff};

//only first 81 + 27 = 108 bits set
const t_128 fsss2::mask108 = {0xFFFFFFFFFFFFFFFF,0xFFFFFFFFFFF};

const t_128 fsss2::visibleCells[81] = { //1 for all 20 visible cells, 1 for the cell itself, 1 for the three houses
{0x80402010081C0FFF,0x0000000804020100},
{0x00804020101C0FFF,0x0000000808020201},
{0x01008040201C0FFF,0x0000000810020402},
{0x0201008040E071FF,0x0000001020020804},
{0x0402010080E071FF,0x0000001040021008},
{0x0804020100E071FF,0x0000001080022010},
{0x10080402070381FF,0x0000002100024020},
{0x20100804070381FF,0x0000002200028040},
{0x40201008070381FF,0x0000002400030080},
{0x80402010081FFE07,0x0000000804040100},
{0x00804020101FFE07,0x0000000808040201},
{0x01008040201FFE07,0x0000000810040402},
{0x0201008040E3FE38,0x0000001020040804},
{0x0402010080E3FE38,0x0000001040041008},
{0x0804020100E3FE38,0x0000001080042010},
{0x100804020703FFC0,0x0000002100044020},
{0x201008040703FFC0,0x0000002200048040},
{0x402010080703FFC0,0x0000002400050080},
{0x804020100FFC0E07,0x0000000804080100},
{0x0080402017FC0E07,0x0000000808080201},
{0x0100804027FC0E07,0x0000000810080402},
{0x0201008047FC7038,0x0000001020080804},
{0x0402010087FC7038,0x0000001040081008},
{0x0804020107FC7038,0x0000001080082010},
{0x1008040207FF81C0,0x0000002100084020},
{0x2010080407FF81C0,0x0000002200088040},
{0x4020100807FF81C0,0x0000002400090080},
{0x8040E07FF8040201,0x0000004004100100},
{0x0080E07FF8080402,0x0000004008100201},
{0x0100E07FF8100804,0x0000004010100402},
{0x0207038FF8201008,0x0000008020100804},
{0x0407038FF8402010,0x0000008040101008},
{0x0807038FF8804020,0x0000008080102010},
{0x10381C0FF9008040,0x0000010100104020},
{0x20381C0FFA010080,0x0000010200108040},
{0x40381C0FFC020100,0x0000010400110080},
{0x8040FFF038040201,0x0000004004200100},
{0x0080FFF038080402,0x0000004008200201},
{0x0100FFF038100804,0x0000004010200402},
{0x02071FF1C0201008,0x0000008020200804},
{0x04071FF1C0402010,0x0000008040201008},
{0x08071FF1C0804020,0x0000008080202010},
{0x10381FFE01008040,0x0000010100204020},
{0x20381FFE02010080,0x0000010200208040},
{0x40381FFE04020100,0x0000010400210080},
{0x807FE07038040201,0x0000004004400100},
{0x00BFE07038080402,0x0000004008400201},
{0x013FE07038100804,0x0000004010400402},
{0x023FE381C0201008,0x0000008020400804},
{0x043FE381C0402010,0x0000008040401008},
{0x083FE381C0804020,0x0000008080402010},
{0x103FFC0E01008040,0x0000010100404020},
{0x203FFC0E02010080,0x0000010200408040},
{0x403FFC0E04020100,0x0000010400410080},
{0xFFC0201008040201,0x0000020004800703},
{0xFFC0402010080402,0x0000020008800703},
{0xFFC0804020100804,0x0000020010800703},
{0x7FC1008040201008,0x000004002080381C},
{0x7FC2010080402010,0x000004004080381C},
{0x7FC4020100804020,0x000004008080381C},
{0x7FC8040201008040,0x000008010081C0E0},
{0x7FD0080402010080,0x000008020081C0E0},
{0x7FE0100804020100,0x000008040081C0E0},
{0x81C0201008040201,0x00000200050007FF},
{0x81C0402010080402,0x00000200090007FF},
{0x81C0804020100804,0x00000200110007FF},
{0x8E01008040201008,0x00000400210038FF},
{0x8E02010080402010,0x00000400410038FF},
{0x8E04020100804020,0x00000400810038FF},
{0xF008040201008040,0x000008010101C0FF},
{0xF010080402010080,0x000008020101C0FF},
{0xF020100804020100,0x000008040101C0FF},
{0x81C0201008040201,0x000002000601FF03},
{0x81C0402010080402,0x000002000A01FF03},
{0x81C0804020100804,0x000002001201FF03},
{0x0E01008040201008,0x000004002201FF1C},
{0x0E02010080402010,0x000004004201FF1C},
{0x0E04020100804020,0x000004008201FF1C},
{0x7008040201008040,0x000008010201FFE0},
{0x7010080402010080,0x000008020201FFE0},
{0x7020100804020100,0x000008040201FFE0},
}; //bm128 visibleCells[81]
/*
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
*/

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
//const t_128 fsss2::houseBits[27] = { //1 for the 9 cells in the house + 1 in the house marker at 81 + houseindex
//	{0x00000000000001FF,0x0000000000020000},
//	{0x000000000003FE00,0x0000000000040000},
//	{0x0000000007FC0000,0x0000000000080000},
//	{0x0000000FF8000000,0x0000000000100000},
//	{0x00001FF000000000,0x0000000000200000},
//	{0x003FE00000000000,0x0000000000400000},
//	{0x7FC0000000000000,0x0000000000800000},
//	{0x8000000000000000,0x00000000010000FF},
//	{0x0000000000000000,0x000000000201FF00},
//	{0x8040201008040201,0x0000000004000100},
//	{0x0080402010080402,0x0000000008000201},
//	{0x0100804020100804,0x0000000010000402},
//	{0x0201008040201008,0x0000000020000804},
//	{0x0402010080402010,0x0000000040001008},
//	{0x0804020100804020,0x0000000080002010},
//	{0x1008040201008040,0x0000000100004020},
//	{0x2010080402010080,0x0000000200008040},
//	{0x4020100804020100,0x0000000400010080},
//	{0x00000000001C0E07,0x0000000800000000},
//	{0x0000000000E07038,0x0000001000000000},
//	{0x00000000070381C0,0x0000002000000000},
//	{0x0000E07038000000,0x0000004000000000},
//	{0x00070381C0000000,0x0000008000000000},
//	{0x00381C0E00000000,0x0000010000000000},
//	{0x81C0000000000000,0x0000020000000703},
//	{0x0E00000000000000,0x000004000000381C},
//	{0x7000000000000000,0x000008000001C0E0},
//}; //houseBits[27]

#ifdef USE_LOCKED_CANDIDATES
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
#endif

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
		//clearSolved(); //unnecessary when calling just before hunting for naked singles
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
//const char* puzzle;

unsigned long long fsss2::solve(const char* const in, const unsigned long long nSolutions, char* const out) {
	//puzzle = in;
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
	//clearSolved(); //unnecessary just before hunting for naked singles
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
	solved.clear(); //no solved cells yet
	//no processed digits for hidden singles yet
	knownNoHiddenSingles[0] = mask108;
	knownNoHiddenSingles[1] = mask108;
	knownNoHiddenSingles[2] = mask108;
	knownNoHiddenSingles[3] = mask108;
	knownNoHiddenSingles[4] = mask108;
	knownNoHiddenSingles[5] = mask108;
	knownNoHiddenSingles[6] = mask108;
	knownNoHiddenSingles[7] = mask108;
	knownNoHiddenSingles[8] = mask108;

#ifdef USE_LOCKED_CANDIDATES
	knownNoLockedCandidates[0] = mask108;
	knownNoLockedCandidates[1] = mask108;
	knownNoLockedCandidates[2] = mask108;
	knownNoLockedCandidates[3] = mask108;
	knownNoLockedCandidates[4] = mask108;
	knownNoLockedCandidates[5] = mask108;
	knownNoLockedCandidates[6] = mask108;
	knownNoLockedCandidates[7] = mask108;
	knownNoLockedCandidates[8] = mask108;
	lockedDone = 0;
#endif
	mode = 0; //should solve
	guessDepth = 0; //no guessed cells yet
}

#ifdef USE_LOCKED_CANDIDATES
void fsss2::doLockedCandidatesForDigit(bm128& tmp) {
	//int houses = 0x03FFFF & ((tmp.toInt64_1()) >> (81 - 64));
	//for (int hbm = houses & -houses; houses; hbm = houses & -houses) {
	for(uint32_t houses = 0x03FFFF & ((tmp.toInt64_1()) >> (81 - 64)); houses; houses &= (houses - 1)) {
		unsigned int rc = 3U * __builtin_ctz(houses); //unsolved row or column
		//process the 3 triplets in the row/col
		for(unsigned int t = 0; t < 3; t++) {
			bool dl = tmp.isDisjoint(tripletMasks[rc + t].adjacentLine);
			bool db = tmp.isDisjoint(tripletMasks[rc + t].adjacentBox);
			if(dl) {
				if(!db) {
					tmp.clearBits(tripletMasks[rc + t].adjacentBox);
					return;
				}
			}
			if(db) {
				if(!dl) {
					if(!tmp.isDisjoint(tripletMasks[rc + t].self)) { //unsolved box
						tmp.clearBits(tripletMasks[rc + t].adjacentLine);
						return;
					}
				}
			}
		} //triplets in a house
	} //houses
}
#endif

//extern int nNaked[129];
//void fsss2::doNakedSingles(bm128& g0, bm128& g1, bm128& g2, bm128& g3, bm128& g4, bm128& g5, bm128& g6, bm128& g7, bm128& g8, bm128& slv) { //cells with only one remaining candidate
void fsss2::doNakedSingles() { //cells with only one remaining candidate
//    __asm__
//    (
//			"":::"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7","%xmm8","%xmm9","%xmm10","%xmm11","%xmm12","xmm13","xmm14","xmm15"
//    );
    bm128& slv = solved;
//#define slv solved
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

	//pushing "solved" to register doesn't work for icc due to register pressure
	//bool changed = false;
	//char* const s = sol;
#define s sol

againNaked:
	//__builtin_prefetch(grid);
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
		if(((bm128)mask81).isSubsetOf(all)) {
			;
		}
		else {
			//there is at least one unsolved cell without any candidate
			mode = MODE_STOP_PROCESSING;
			return;
		}
		if(((bm128)mask81).isSubsetOf(duplicates)) {
			//sorry, no naked singles
			//remove all candidates for the solved cells (even if no eliminations were made here)
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
		all = mask81;
		all.clearBits(duplicates);
		slv |= all; //mark cells as solved
	}
	//now find which unique where came from
	for(uint64_t cells = all.toInt64(); cells; cells &= (cells - 1)) {
		uint64_t cell = bm128::FindLSBIndex64(cells); //get the rightmost bit index
		__builtin_prefetch(bitSet + cell);
		const bm128 theCells = visibleCells[cell];
		const bm128 theBit = bitSet[cell];
		if(s) {
			if(!theBit.isSubsetOf(g0)) ; else {g0.clearBits(theCells); s[cell] = 1; continue;}
			if(!theBit.isSubsetOf(g1)) ; else {g1.clearBits(theCells); s[cell] = 2; continue;}
			if(!theBit.isSubsetOf(g2)) ; else {g2.clearBits(theCells); s[cell] = 3; continue;}
			if(!theBit.isSubsetOf(g3)) ; else {g3.clearBits(theCells); s[cell] = 4; continue;}
			if(!theBit.isSubsetOf(g4)) ; else {g4.clearBits(theCells); s[cell] = 5; continue;}
			if(!theBit.isSubsetOf(g5)) ; else {g5.clearBits(theCells); s[cell] = 6; continue;}
			if(!theBit.isSubsetOf(g6)) ; else {g6.clearBits(theCells); s[cell] = 7; continue;}
			if(!theBit.isSubsetOf(g7)) ; else {g7.clearBits(theCells); s[cell] = 8; continue;}
			if(!theBit.isSubsetOf(g8)) ; else {g8.clearBits(theCells); s[cell] = 9; continue;}
		}
		else {
			if(!theBit.isSubsetOf(g0)) ; else {g0.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g1)) ; else {g1.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g2)) ; else {g2.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g3)) ; else {g3.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g4)) ; else {g4.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g5)) ; else {g5.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g6)) ; else {g6.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g7)) ; else {g7.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g8)) ; else {g8.clearBits(theCells); continue;}
		}
		//this cell has been just cleared by setting other naked single (2 naked in a house for the same digit)
		//now this cell has no candidates which is a contradiction
		mode = MODE_STOP_PROCESSING;
		return;
	} //for lower 64 cells
	for(uint32_t cells = all.toInt32_2(); cells; cells &= (cells - 1)) {
		unsigned int cell = 64 + bm128::FindLSBIndex32(cells); //get the rightmost bit index
		__builtin_prefetch(bitSet + cell);
		const bm128 theCells = visibleCells[cell];
		const bm128 theBit = bitSet[cell];
		if(s) {
			if(!theBit.isSubsetOf(g0)) ; else {g0.clearBits(theCells); s[cell] = 1; continue;}
			if(!theBit.isSubsetOf(g1)) ; else {g1.clearBits(theCells); s[cell] = 2; continue;}
			if(!theBit.isSubsetOf(g2)) ; else {g2.clearBits(theCells); s[cell] = 3; continue;}
			if(!theBit.isSubsetOf(g3)) ; else {g3.clearBits(theCells); s[cell] = 4; continue;}
			if(!theBit.isSubsetOf(g4)) ; else {g4.clearBits(theCells); s[cell] = 5; continue;}
			if(!theBit.isSubsetOf(g5)) ; else {g5.clearBits(theCells); s[cell] = 6; continue;}
			if(!theBit.isSubsetOf(g6)) ; else {g6.clearBits(theCells); s[cell] = 7; continue;}
			if(!theBit.isSubsetOf(g7)) ; else {g7.clearBits(theCells); s[cell] = 8; continue;}
			if(!theBit.isSubsetOf(g8)) ; else {g8.clearBits(theCells); s[cell] = 9; continue;}
		}
		else {
			if(!theBit.isSubsetOf(g0)) ; else {g0.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g1)) ; else {g1.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g2)) ; else {g2.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g3)) ; else {g3.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g4)) ; else {g4.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g5)) ; else {g5.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g6)) ; else {g6.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g7)) ; else {g7.clearBits(theCells); continue;}
			if(!theBit.isSubsetOf(g8)) ; else {g8.clearBits(theCells); continue;}
		}
		//this cell has been just cleared by setting other naked single (2 naked in a house for the same digit)
		//now this cell has no candidates which is a contradiction
		mode = MODE_STOP_PROCESSING;
		return;
	} //for upper 17 cells
	if(!((bm128)mask81).isSubsetOf(slv)) {
		//changed = true;
		goto againNaked;
	}
	//finally all 81 cells are solved
	solutionFound();
	return;
}

//inline void fsss2::clearSolved() {
//	bm128 tmp = solved;
//	grid[0].clearBits(tmp);
//	grid[1].clearBits(tmp);
//	grid[2].clearBits(tmp);
//	grid[3].clearBits(tmp);
//	grid[4].clearBits(tmp);
//	grid[5].clearBits(tmp);
//	grid[6].clearBits(tmp);
//	grid[7].clearBits(tmp);
//	grid[8].clearBits(tmp);
//}

void fsss2::doEliminations() {
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
				for(uint32_t houses = /*((1 << 27) - 1) &*/(grid[d].toInt64_1()) >> (81 - 64); houses; houses &= (houses - 1)) {
					bm128 tmp = grid[d];
					//static const t_128 minus1 = {0xffffffffffffffff,0xffffffffffffffff};
					//__builtin_prefetch(&minus1);
					{
						//unsigned int j = bm128::FindLSBIndex32(houses);
						//tmp &= bitsForHouse[j]; //mask other candidates and leave only these from the current house
						tmp &= bitsForHouse[bm128::FindLSBIndex32(houses)]; //mask other candidates and leave only these from the current house
					}
					//find whether the house has a single candidate and obtain its position
					uint64_t cell;
					//exploit the fact that when (x & (x-1)) == 0 then x has 0 or 1 bits set
					//static const t_128 minus1 = {0xffffffffffffffff,0xffffffffffffffff};
					if(0 == _mm_testz_si128(tmp.bitmap128.m128i_m128i, _mm_add_epi64(tmp.bitmap128.m128i_m128i, minus1.m128i_m128i)))
					//if(0 == tmp.hasMax2Bits())
						continue; //too many candidates
					//find the bit
					{
						uint64_t low64 = tmp.toInt64();
						uint32_t high17 = tmp.toInt32_2();
						if(low64) {
							if(high17) continue; //candidates in both low and high part of the house
							//get the position of the single candidate in the low part of the house
							cell = bm128::FindLSBIndex64(low64);
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
					if(sol)
						sol[cell] = d + 1; //store the digit if solution buffer is given
					solved.setBit(cell); //mark cell as "solved"
					grid[d].clearBits(visibleCells[cell]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
					//at this point the solved cell still isn't cleared from all 9 masks, but doNakedSingles does it
					doNakedSingles(); //checking a single cell, possible eliminations in other digits
					if(mode) goto backtrack;
					found = d; //if the latest found is in the 1-st digit then repeating search for hiddens is redundant
					goto againSameHidden;
				} //for houses
				knownNoHiddenSingles[d] = grid[d];
			}  //for d
		} while(found);
	} //end of hidden singles

	//locked candidates
#ifdef USE_LOCKED_CANDIDATES
	if(lockedDone == 0) {
		int found = 0;
		//if(solved.popcount_128() < 29) {
			//if a digit in a row is within a single triplet, then remove digit from the box triplets and vice versa
			for (int d = 0; d < 9; d++) {
				bm128 tmp = grid[d];
				if(tmp == knownNoLockedCandidates[d])
					continue;
				doLockedCandidatesForDigit(grid[d]);
				if(tmp.isSubsetOf(grid[d])) {
					knownNoLockedCandidates[d] = tmp;
					continue;
				}
				//some eliminations are done for this digit
				found = 1;
				//goto nakedAgain;
			}
		//}
		lockedDone = 1;
		if(found) goto nakedAgain;
	} //end of locked candidates
#endif //USE_LOCKED_CANDIDATES

	//Prepare a guess
	{
		//At this point the existence of unsolved house(s) w/o candidates crashes the algorithm!!!

		//Find an unsolved cell with less possibilities
		int optDigit;
		int optCell;

		//find first bi-value cell and return first of the two values
		findBiValueCell(optDigit, optCell);
		if(optDigit != -1) {
			;
		}
		else {
			//find house with less candidates from a particular digit, exit on first bi-position house/digit
			findBiPositionDigit(optDigit, optCell);
		}

		{
			bm128* gg = &contexts[guessDepth++][0];
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
//			gg[10] = knownNoLockedCandidates[0];
//			gg[11] = knownNoLockedCandidates[1];
//			gg[12] = knownNoLockedCandidates[2];
//			gg[13] = knownNoLockedCandidates[3];
//			gg[14] = knownNoLockedCandidates[4];
//			gg[15] = knownNoLockedCandidates[5];
//			gg[16] = knownNoLockedCandidates[6];
//			gg[17] = knownNoLockedCandidates[7];
//			gg[18] = knownNoLockedCandidates[8];
			//later continue with this candidate eliminated
			gg[optDigit].clearBit(optCell);
			//try the "optimal" cell/digit candidate
			if(sol)
				sol[optCell] = optDigit + 1; //store the digit if solution buffer is given
			solved.setBit(optCell); //mark cell as "solved"
			grid[optDigit].clearBits(visibleCells[optCell]); //mark visible cells as forbidden for the same digit, mark the 3 houses as solved
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
//		knownNoLockedCandidates[0] = gg[10];
//	    knownNoLockedCandidates[1] = gg[11];
//	    knownNoLockedCandidates[2] = gg[12];
//	    knownNoLockedCandidates[3] = gg[13];
//	    knownNoLockedCandidates[4] = gg[14];
//	    knownNoLockedCandidates[5] = gg[15];
//	    knownNoLockedCandidates[6] = gg[16];
//	    knownNoLockedCandidates[7] = gg[17];
//	    knownNoLockedCandidates[8] = gg[18];
		mode = 0;
	}
	goto nakedAgain;
}

void fsss2::findBiValueCells(bm128& all) const { //cells with 2 remaining candidates
	//bm128 all;
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
}

//inline void fsss2::findBiValueCell(int& digit, int& cell, int& digit2, bm128& all) const { //cells with 2 remaining candidates
void fsss2::findBiValueCell(int& digit, int& cell) const { //cells with 2 remaining candidates
//inline void fsss2::findBiValueCell(int& digit, int& cell, int& digit2) const { //cells with 2 remaining candidates
	bm128 all;
	findBiValueCells(all);
	if(all.isZero()) {
		digit = -1;
		return;
	}
//	for(int d = 0; d < 8; d++) {
//		if(!all.isDisjoint(grid[d])) {
//			all &= grid[d];
//			cell = all.getFirstBit1Index96();
//			digit = d;
//			for(int d2 = d + 1; d2 < 9; d2++) {
//				if(grid[d2].isBitSet(cell)) {
//					digit2 = d2;
//					return;
//				}
//			}
//		}
//	}
	for(digit = 0; digit < 8; digit++) {
		if(!all.isDisjoint(grid[digit])) {
			all &= grid[digit];
			cell = all.getFirstBit1Index96();
			break;
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

void fsss2::findBiPositionDigit(int& digit, int& cell) const {
	//find house with less candidates from a particular digit, exit on first bi-position house/digit
	int minCells = 100;
	for(int d = 0; d < 9; d++) {
		for(int h = 0; h < 27; h++) {
			if(!grid[d].isBitSet(81 + h))
				continue;
			bm128 tmp = grid[d];
			tmp &= bitsForHouse[h];
			tmp &= mask81;
			int n = tmp.popcount_128();
			if(n < minCells) {
				cell = tmp.getFirstBit1Index96();
				digit = d;
				if(n == 2) {
					return; //not so bad, a bi-position is found
				}
				minCells = n;
			}
		}
	}
	//for(int i = 0; i < 81; i++)
	//	printf("%c", puzzle[i] + '0');
	//printf("\t%d\n", minCells);
}
