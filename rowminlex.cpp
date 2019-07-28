//Code inspired by Glenn Fowler's sudoku solver/generator
//The original is published under the following license
		/***********************************************************************
		*               This software is part of the ast package               *
		*          Copyright (c) 2005-2009 AT&T Intellectual Property          *
		*                      and is licensed under the                       *
		*                  Common Public License, Version 1.0                  *
		*                    by AT&T Intellectual Property                     *
		*                                                                      *
		*                A copy of the License is available at                 *
		*            http://www.opensource.org/licenses/cpl1.0.txt             *
		*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
		*                                                                      *
		*              Information and Software Systems Research               *
		*                            AT&T Research                             *
		*                           Florham Park NJ                            *
		*                                                                      *
		*                 Glenn Fowler <gsf@research.att.com>                  *
		***********************************************************************/
//This is a MODIFIED version of the original code

#include <memory.h>
#include "rowminlex.h"

const transformer::transformationConstants transformer::tc =
{
	{ /* swap */
		{ // 0
			{  0, 1, 2, 3, 4, 5, 6, 7, 8 }, // 123
			{  9,10,11,12,13,14,15,16,17 }, // 456
			{ 18,19,20,21,22,23,24,25,26 }, // 789
			{ 27,28,29,30,31,32,33,34,35 },
			{ 36,37,38,39,40,41,42,43,44 },
			{ 45,46,47,48,49,50,51,52,53 },
			{ 54,55,56,57,58,59,60,61,62 },
			{ 63,64,65,66,67,68,69,70,71 },
			{ 72,73,74,75,76,77,78,79,80 }
		},
		{ // 1
			{  3, 4, 5, 6, 7, 8, 0, 1, 2 }, // 231
			{ 12,13,14,15,16,17, 9,10,11 }, // 564
			{ 21,22,23,24,25,26,18,19,20 }, // 897
			{ 30,31,32,33,34,35,27,28,29 },
			{ 39,40,41,42,43,44,36,37,38 },
			{ 48,49,50,51,52,53,45,46,47 },
			{ 57,58,59,60,61,62,54,55,56 },
			{ 66,67,68,69,70,71,63,64,65 },
			{ 75,76,77,78,79,80,72,73,74 }
		},
		{ // 2
			{  6, 7, 8, 0, 1, 2, 3, 4, 5 }, // 312
			{ 15,16,17, 9,10,11,12,13,14 }, // 645
			{ 24,25,26,18,19,20,21,22,23 }, // 978
			{ 33,34,35,27,28,29,30,31,32 },
			{ 42,43,44,36,37,38,39,40,41 },
			{ 51,52,53,45,46,47,48,49,50 },
			{ 60,61,62,54,55,56,57,58,59 },
			{ 69,70,71,63,64,65,66,67,68 },
			{ 78,79,80,72,73,74,75,76,77 }
		},
		{ // 3
			{ 27,28,29,30,31,32,33,34,35 }, // 456
			{ 36,37,38,39,40,41,42,43,44 }, // 789
			{ 45,46,47,48,49,50,51,52,53 }, // 123
			{ 54,55,56,57,58,59,60,61,62 },
			{ 63,64,65,66,67,68,69,70,71 },
			{ 72,73,74,75,76,77,78,79,80 },
			{  0, 1, 2, 3, 4, 5, 6, 7, 8 },
			{  9,10,11,12,13,14,15,16,17 },
			{ 18,19,20,21,22,23,24,25,26 }
		},
		{ // 4
			{ 30,31,32,33,34,35,27,28,29 }, // 564
			{ 39,40,41,42,43,44,36,37,38 }, // 897
			{ 48,49,50,51,52,53,45,46,47 }, // 231
			{ 57,58,59,60,61,62,54,55,56 },
			{ 66,67,68,69,70,71,63,64,65 },
			{ 75,76,77,78,79,80,72,73,74 },
			{  3, 4, 5, 6, 7, 8, 0, 1, 2 },
			{ 12,13,14,15,16,17, 9,10,11 },
			{ 21,22,23,24,25,26,18,19,20 }
		},
		{ // 5
			{ 33,34,35,27,28,29,30,31,32 }, // 645
			{ 42,43,44,36,37,38,39,40,41 }, // 978
			{ 51,52,53,45,46,47,48,49,50 }, // 312
			{ 60,61,62,54,55,56,57,58,59 },
			{ 69,70,71,63,64,65,66,67,68 },
			{ 78,79,80,72,73,74,75,76,77 },
			{  6, 7, 8, 0, 1, 2, 3, 4, 5 },
			{ 15,16,17, 9,10,11,12,13,14 },
			{ 24,25,26,18,19,20,21,22,23 }
		},
		{ // 6
			{ 54,55,56,57,58,59,60,61,62 }, // 789
			{ 63,64,65,66,67,68,69,70,71 }, // 123
			{ 72,73,74,75,76,77,78,79,80 }, // 456
			{  0, 1, 2, 3, 4, 5, 6, 7, 8 },
			{  9,10,11,12,13,14,15,16,17 },
			{ 18,19,20,21,22,23,24,25,26 },
			{ 27,28,29,30,31,32,33,34,35 },
			{ 36,37,38,39,40,41,42,43,44 },
			{ 45,46,47,48,49,50,51,52,53 }
		},
		{ // 7
			{ 57,58,59,60,61,62,54,55,56 }, // 897
			{ 66,67,68,69,70,71,63,64,65 }, // 231
			{ 75,76,77,78,79,80,72,73,74 }, // 564
			{  3, 4, 5, 6, 7, 8, 0, 1, 2 },
			{ 12,13,14,15,16,17, 9,10,11 },
			{ 21,22,23,24,25,26,18,19,20 },
			{ 30,31,32,33,34,35,27,28,29 },
			{ 39,40,41,42,43,44,36,37,38 },
			{ 48,49,50,51,52,53,45,46,47 }
		},
		{ // 8
			{ 60,61,62,54,55,56,57,58,59 }, // 978
			{ 69,70,71,63,64,65,66,67,68 }, // 312
			{ 78,79,80,72,73,74,75,76,77 }, // 645
			{  6, 7, 8, 0, 1, 2, 3, 4, 5 },
			{ 15,16,17, 9,10,11,12,13,14 },
			{ 24,25,26,18,19,20,21,22,23 },
			{ 33,34,35,27,28,29,30,31,32 },
			{ 42,43,44,36,37,38,39,40,41 },
			{ 51,52,53,45,46,47,48,49,50 }
		},
		{ // 9 = transpose(0)
			{  0, 9,18,27,36,45,54,63,72 }, // 147
			{  1,10,19,28,37,46,55,64,73 }, // 258
			{  2,11,20,29,38,47,56,65,74 }, // 369
			{  3,12,21,30,39,48,57,66,75 },
			{  4,13,22,31,40,49,58,67,76 },
			{  5,14,23,32,41,50,59,68,77 },
			{  6,15,24,33,42,51,60,69,78 },
			{  7,16,25,34,43,52,61,70,79 },
			{  8,17,26,35,44,53,62,71,80 }
		},
		{ // 10
			{ 27,36,45,54,63,72, 0, 9,18 }, // 471
			{ 28,37,46,55,64,73, 1,10,19 }, // 582
			{ 29,38,47,56,65,74, 2,11,20 }, // 693
			{ 30,39,48,57,66,75, 3,12,21 },
			{ 31,40,49,58,67,76, 4,13,22 },
			{ 32,41,50,59,68,77, 5,14,23 },
			{ 33,42,51,60,69,78, 6,15,24 },
			{ 34,43,52,61,70,79, 7,16,25 },
			{ 35,44,53,62,71,80, 8,17,26 }
		},
		{ // 11
			{ 54,63,72, 0, 9,18,27,36,45 }, // 714
			{ 55,64,73, 1,10,19,28,37,46 }, // 825
			{ 56,65,74, 2,11,20,29,38,47 }, // 936
			{ 57,66,75, 3,12,21,30,39,48 },
			{ 58,67,76, 4,13,22,31,40,49 },
			{ 59,68,77, 5,14,23,32,41,50 },
			{ 60,69,78, 6,15,24,33,42,51 },
			{ 61,70,79, 7,16,25,34,43,52 },
			{ 62,71,80, 8,17,26,35,44,53 }
		},
		{ // 12
			{  3,12,21,30,39,48,57,66,75 }, // 258
			{  4,13,22,31,40,49,58,67,76 }, // 369
			{  5,14,23,32,41,50,59,68,77 }, // 147
			{  6,15,24,33,42,51,60,69,78 },
			{  7,16,25,34,43,52,61,70,79 },
			{  8,17,26,35,44,53,62,71,80 },
			{  0, 9,18,27,36,45,54,63,72 },
			{  1,10,19,28,37,46,55,64,73 },
			{  2,11,20,29,38,47,56,65,74 }
		},
		{ // 13
			{ 30,39,48,57,66,75, 3,12,21 },
			{ 31,40,49,58,67,76, 4,13,22 },
			{ 32,41,50,59,68,77, 5,14,23 },
			{ 33,42,51,60,69,78, 6,15,24 },
			{ 34,43,52,61,70,79, 7,16,25 },
			{ 35,44,53,62,71,80, 8,17,26 },
			{ 27,36,45,54,63,72, 0, 9,18 },
			{ 28,37,46,55,64,73, 1,10,19 },
			{ 29,38,47,56,65,74, 2,11,20 }
		},
		{ // 14
			{ 57,66,75, 3,12,21,30,39,48 },
			{ 58,67,76, 4,13,22,31,40,49 },
			{ 59,68,77, 5,14,23,32,41,50 },
			{ 60,69,78, 6,15,24,33,42,51 },
			{ 61,70,79, 7,16,25,34,43,52 },
			{ 62,71,80, 8,17,26,35,44,53 },
			{ 54,63,72, 0, 9,18,27,36,45 },
			{ 55,64,73, 1,10,19,28,37,46 },
			{ 56,65,74, 2,11,20,29,38,47 }
		},
		{ // 15
			{  6,15,24,33,42,51,60,69,78 }, // 369
			{  7,16,25,34,43,52,61,70,79 }, // 147
			{  8,17,26,35,44,53,62,71,80 }, // 258
			{  0, 9,18,27,36,45,54,63,72 },
			{  1,10,19,28,37,46,55,64,73 },
			{  2,11,20,29,38,47,56,65,74 },
			{  3,12,21,30,39,48,57,66,75 },
			{  4,13,22,31,40,49,58,67,76 },
			{  5,14,23,32,41,50,59,68,77 }
		},
		{
			{ 33,42,51,60,69,78, 6,15,24 },
			{ 34,43,52,61,70,79, 7,16,25 },
			{ 35,44,53,62,71,80, 8,17,26 },
			{ 27,36,45,54,63,72, 0, 9,18 },
			{ 28,37,46,55,64,73, 1,10,19 },
			{ 29,38,47,56,65,74, 2,11,20 },
			{ 30,39,48,57,66,75, 3,12,21 },
			{ 31,40,49,58,67,76, 4,13,22 },
			{ 32,41,50,59,68,77, 5,14,23 }
		},
		{
			{ 60,69,78, 6,15,24,33,42,51 },
			{ 61,70,79, 7,16,25,34,43,52 },
			{ 62,71,80, 8,17,26,35,44,53 },
			{ 54,63,72, 0, 9,18,27,36,45 },
			{ 55,64,73, 1,10,19,28,37,46 },
			{ 56,65,74, 2,11,20,29,38,47 },
			{ 57,66,75, 3,12,21,30,39,48 },
			{ 58,67,76, 4,13,22,31,40,49 },
			{ 59,68,77, 5,14,23,32,41,50 }
		},
	},
	{ /* perm */
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 1, 0, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
		{ 2, 1, 0 }
	},
	{ /* part */
		{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0 },
		{ 4, 5, 6, 7, 8 },
		{ 3, 5, 6, 7, 8 },
		{ 3, 4, 6, 7, 8 },
		{ 7, 8, 3, 4, 5 },
		{ 6, 8, 3, 4, 5 },
		{ 6, 7, 3, 4, 5 }
	},
	{ /* boxOffset */
		0, 0, 0, 3, 3, 3, 6, 6, 6
	}
};

void transformer::byGrid(const char* sol)
{
	register int			b;
	register int			d;
	register int			i;
	register int			j;
	register int			k;
	register int			l;
	register unsigned int	v;

	int				pr;
	int				pc;

	transformer test;

	/* initialize the worst canonical candidate */
	aut = 1;
	box = 0;
	for(i = 0; i < 9; i++) {
		map[i] = 10;
		row[i] = col[i] = i;
	}
	map[9] = 10;

	/* search over all boxes */

	for(b = 0; b < 18; b++) {
		for(pr = 0; pr < 6; pr++) {
			for(pc = 0; pc < 6; pc++) {
				test.box = b;

				/* initialize the map */

				for(i = 0; i < 3; i++) {
					test.row[i] = tc.perm[pr][i];
					test.col[i] = tc.perm[pc][i];
				}
				for(i = 1; i <= 9; i++)
					test.map[i] = 0;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[0]]]] = 1;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[1]]]] = 2;
				test.map[(int)sol[tc.swap[b][test.row[0]][test.col[2]]]] = 3;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[0]]]] = 4;
				test.map[(int)sol[tc.swap[b][test.row[1]][test.col[1]]]] = 5;

				/* map value 6 and order cols 4&5&6 */

				k = 0;
				for(i = 3; i < 9; i++) {
					if(test.map[(int)sol[tc.swap[b][test.row[0]][i]]] == 4) {
						l = tc.boxOffset[i];
						for(j = 0; j < 3; j++) {
							switch (test.map[(int)sol[tc.swap[b][test.row[0]][j+l]]])
							{
							case 4:
								test.col[3] = j+l;
								k |= 01;
								break;
							case 5:
								test.col[4] = j+l;
								k |= 02;
								break;
							case 0:
								test.map[(int)sol[tc.swap[b][test.row[0]][test.col[5]=j+l]]] = 6;
								k |= 04;
								break;
							}
						}
						break;
					}
				}
				if(k != 7)
					goto next;

				/* map values 7&8&9 */

				for(j = 2; j < 6; j++) {
					if(!test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]]) {
						test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] = k++;
					}
				}

				/* check row 2 cols 3&4&5&6 */

				for(j = 2; j < 6; j++) {
					if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
						if(d > 0) {
							goto next;
						}
						break;
					}
				}

				/* order cols 7&8&9 */

				j = (l == 3) ? 6 : 3;
				k = j+1;
				l = j+2;
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][k]]]) {
					v = j; j = k; k = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][j]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = j; j = l; l = v;
				}
				if(test.map[(int)sol[tc.swap[b][test.row[0]][k]]] > test.map[(int)sol[tc.swap[b][test.row[0]][l]]]) {
					v = k; k = l; l = v;
				}
				test.col[6] = j;
				test.col[7] = k;
				test.col[8] = l;

				/* check row 2 cols 7&8&9 */

				if(!d) {
					for(j = 6; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[1]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[1]][col[j]]]])) {
							if(d > 0)
								goto next;
							break;
						}
					}
				}

				/* check row 3 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][test.row[2]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[2]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}

				/* done with the first band */

				/* order row 4 */

				v = 10;
				i = 9;
				while (--i >= 3) {
					if(test.map[(int)sol[tc.swap[b][i][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][i][test.col[0]]]];
						k = i;
					}
				}

				/* check row 4 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][k][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[3]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[3] = k;

				/* order rows 5&6 */

				if(test.map[(int)sol[tc.swap[b][tc.part[k][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[k][1]][test.col[0]]]]) {
					test.row[4] = tc.part[k][0];
					test.row[5] = tc.part[k][1];
				}
				else {
					test.row[4] = tc.part[k][1];
					test.row[5] = tc.part[k][0];
				}

				/* check rows 5&6 */

				if(!d) {
					for(i = 4; i < 6; i++) {
						for(j = 0; j < 9; j++) {
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 6;
								break;
							}
						}
					}
				}

				/* done with the second band */

				/* order row 7 */

				v = 10;
				for(i = 2; i < 5; i++) {
					if(test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]] < v) {
						v = test.map[(int)sol[tc.swap[b][tc.part[k][i]][test.col[0]]]];
						l = tc.part[k][i];
					}
				}

				/* check row 7 */

				if(!d) {
					for(j = 0; j < 9; j++) {
						if((d = (int)test.map[(int)sol[tc.swap[b][l][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[6]][col[j]]]])) {
							if(d > 0) {
								goto next;
							}
							break;
						}
					}
				}
				test.row[6] = l;

				/* order rows 8&9 */

				if(test.map[(int)sol[tc.swap[b][tc.part[l][0]][test.col[0]]]] < test.map[(int)sol[tc.swap[b][tc.part[l][1]][test.col[0]]]]) {
					test.row[7] = tc.part[l][0];
					test.row[8] = tc.part[l][1];
				}
				else {
					test.row[7] = tc.part[l][1];
					test.row[8] = tc.part[l][0];
				}

				/* check rows 8&9 */

				if(!d) {
					for(i = 7; i < 9; i++)
						for(j = 0; j < 9; j++)
							if((d = (int)test.map[(int)sol[tc.swap[b][test.row[i]][test.col[j]]]] - (int)map[(int)sol[tc.swap[box][row[i]][col[j]]]])) {
								if(d > 0) {
									goto next;
								}
								i = 9;
								break;
							}
				}

				/* check for automorphism */

				if(d) {
					clearAutomorphs();
					*this = test;
					aut = 1;
				}
				else {
					addAutomorph(&test);
					aut++;
				}
next:
				;
			}
		}
	}
	map[0] = 0;
}

void transformer::transform(const pencilmarks& in, pencilmarks& out) const {
	for(int d = 0; d < 9; d++) {
		out[d].clear();
	}
	for(int r = 0, destCell = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++, destCell++) {
			int srcCell = tc.swap[box][row[r]][col[c]];
			for(int d = 0; d < 9; d++) {
				if(in[d].isBitSet(srcCell)) {
					out[map[d + 1] - 1].setBit(destCell);
				}
			}
		}
	}
	if(next) {
		//transforming a puzzle of automorphic grid
		pencilmarks aout;
		next->transform(in, aout); //minimal of the rest of the transformations
		if(memcmp(&out, &aout, sizeof(pencilmarks)) > 0) {
			memcpy(&out, &aout, sizeof(pencilmarks));
		}
	}
}

void transformer::transform(const char *in, char *out) const {
	char* s = out;
	bool isPuzzle = false;
	for(int r = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++) {
			char mapped = map[(int)in[tc.swap[box][row[r]][col[c]]]];
			isPuzzle |= (mapped == 0);
			*s++ = mapped;
		}
	}
	if(next && isPuzzle) {
		//transforming a puzzle of automorphic grid
		char aout[81];
		next->transform(in, aout); //minimal of the rest of the transformations
		for(int i = 0; i < 81; i++) {
			if(out[i] > aout[i]) {
				//choose the second omorph
				memcpy(out, aout, 81);
				break;
			}
			else if(out[i] < aout[i]) {
				//choose the first morph
				break;
			}
		}
	}
}

void transformer::reverseTransform(const char *in, char *out) const {
	int labelMap[10];
	for(int i = 0; i < 10; i++) {
		labelMap[map[i]] = i;
	}
	for(int r = 0; r < 9; r++) {
		for(int c = 0; c < 9; c++) {
			out[tc.swap[box][row[r]][col[c]]] = labelMap[(int)in[9 * r + c]];
		}
	}
}

//void rowminlex(const char *in, char *out) {
//	transformer tr;
//	tr.byGrid(in);
//	tr.transform(in, out);
//}

