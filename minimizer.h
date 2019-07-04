#include <inttypes.h>
#include <memory.h>
#include "t_128.h"
#include "fsss2.h"

struct knownNonRedundantGivens {
    uint64_t aliveGivensMask;
    uint64_t knownNonRedundantsMask;
    bool operator< (const knownNonRedundantGivens& givensMask) const {return aliveGivensMask < givensMask.aliveGivensMask;}
    void markAsNonRedundant(int pos) {knownNonRedundantsMask |= ((uint64_t)1 << pos);}
    bool isForRemoval(int pos) const {return (aliveGivensMask & ((uint64_t)1 << pos)) && !(knownNonRedundantsMask & ((uint64_t)1 << pos));}
    void getReducedGivensFrom(const knownNonRedundantGivens &src, int pos) {aliveGivensMask = src.aliveGivensMask & (~((uint64_t)1 << pos));}
};
struct complementaryPencilmarksX {
    bm128 forbiddenValuePositions[9];
    bm128 fixedValuePositions[9];
    bool operator< (const complementaryPencilmarksX& other) const {
    	//return memcmp(forbiddenValuePositions, other.aliveConstrainsMask, sizeof(forbiddenValuePositions) * sizeof(forbiddenValuePositions[0])) > 0;
    	for(int g = 8; g >= 0; g--) {
			if(forbiddenValuePositions[g].bitmap128.m128i_u64[1] < other.forbiddenValuePositions[g].bitmap128.m128i_u64[1]) return true;
			if(forbiddenValuePositions[g].bitmap128.m128i_u64[1] > other.forbiddenValuePositions[g].bitmap128.m128i_u64[1]) return false;
			if(forbiddenValuePositions[g].bitmap128.m128i_u64[0] < other.forbiddenValuePositions[g].bitmap128.m128i_u64[0]) return true;
			if(forbiddenValuePositions[g].bitmap128.m128i_u64[0] > other.forbiddenValuePositions[g].bitmap128.m128i_u64[0]) return false;
    	}
    	return false;
   }
   bool operator== (const complementaryPencilmarksX& constraintsMask) const {
       return memcmp(forbiddenValuePositions, constraintsMask.forbiddenValuePositions, sizeof(forbiddenValuePositions) * sizeof(forbiddenValuePositions[0])) == 0;
   }
    void markAsFixed(int given, int cell) {fixedValuePositions[given].setBit(cell);}
    bool isForRemoval(int given, int cell) const {return (forbiddenValuePositions[given].isBitSet(cell) & !fixedValuePositions[given].isBitSet(cell));}
    void getReducedForbiddensFrom(const complementaryPencilmarksX &src, int given, int cell) {
    	forbiddenValuePositions[0] = src.forbiddenValuePositions[0];
    	forbiddenValuePositions[1] = src.forbiddenValuePositions[1];
    	forbiddenValuePositions[2] = src.forbiddenValuePositions[2];
    	forbiddenValuePositions[3] = src.forbiddenValuePositions[3];
    	forbiddenValuePositions[4] = src.forbiddenValuePositions[4];
    	forbiddenValuePositions[5] = src.forbiddenValuePositions[5];
    	forbiddenValuePositions[6] = src.forbiddenValuePositions[6];
    	forbiddenValuePositions[7] = src.forbiddenValuePositions[7];
    	forbiddenValuePositions[8] = src.forbiddenValuePositions[8];
    	forbiddenValuePositions[given].clearBit(cell);
    }
    void getFixedFrom(const complementaryPencilmarksX &src) {
    	fixedValuePositions[0] = src.fixedValuePositions[0];
    	fixedValuePositions[1] = src.fixedValuePositions[1];
    	fixedValuePositions[2] = src.fixedValuePositions[2];
    	fixedValuePositions[3] = src.fixedValuePositions[3];
    	fixedValuePositions[4] = src.fixedValuePositions[4];
    	fixedValuePositions[5] = src.fixedValuePositions[5];
    	fixedValuePositions[6] = src.fixedValuePositions[6];
    	fixedValuePositions[7] = src.fixedValuePositions[7];
    	fixedValuePositions[8] = src.fixedValuePositions[8];
    }
    bool isMinimal() const {
    	return
    			forbiddenValuePositions[0].isSubsetOf(fixedValuePositions[0]) &&
    			forbiddenValuePositions[1].isSubsetOf(fixedValuePositions[1]) &&
    			forbiddenValuePositions[2].isSubsetOf(fixedValuePositions[2]) &&
    			forbiddenValuePositions[3].isSubsetOf(fixedValuePositions[3]) &&
    			forbiddenValuePositions[4].isSubsetOf(fixedValuePositions[4]) &&
    			forbiddenValuePositions[5].isSubsetOf(fixedValuePositions[5]) &&
    			forbiddenValuePositions[6].isSubsetOf(fixedValuePositions[6]) &&
    			forbiddenValuePositions[7].isSubsetOf(fixedValuePositions[7]) &&
    			forbiddenValuePositions[8].isSubsetOf(fixedValuePositions[8]);
    }
    bool isMinimalUniqueDoubleCheck(const char* sol) const {
		char sol2[88];
		getSingleSolution ss;
		int nSol = ss.solve(forbiddenValuePositions, sol2);
		if(nSol != 1) {
			printf("\nNot unique, nSol = %d\n", nSol);
			return false;
		}
		if(memcmp(sol, sol2, 81) != 0) {
			printf("\nWrong solution\n");
			return false;
		}
		for(int d = 0; d < 9; d++) {
			for(int c = 0; c < 81; c++) {
				if(forbiddenValuePositions[d].isBitSet(c)) {
					//try removal of this pencilmark and see whether it causes 2+ solutions
					complementaryPencilmarksX tmp(*this);
					hasSingleSolution sss;
					tmp.forbiddenValuePositions[d].clearBit(c);
					if(2 != sss.solve(tmp.forbiddenValuePositions)) {
						printf("\nNon-minimal, value=%d, cell=%d is redundant\n", d + 1, c);
						dump();
						return false;
					}
				}
			}
		}
		return true;
	}
    void dump() const {
		//printf("\nforbidden\n");
    	//dump1(forbiddenValuePositions);
		printf("\nallowed\n");
    	dump1(forbiddenValuePositions, true);
		printf("\nfixed\n");
    	dump1(fixedValuePositions);
    }
    static void dump1(const bm128* what, bool invert = false) {
		for(int c = 0; c < 81; c++) {
			if((c + 0) % 9 == 0) {
				printf("|");
			}
			for(int d = 0; d < 9; d++) {
				if(invert != what[d].isBitSet(c)) {
					printf("%d", d + 1);
				}
				else {
					printf(".");
				}
			}
			if((c + 1) % 3 == 0) {
				printf("|");
			}
			else {
				printf(" ");
			}
			if((c + 1) % 9 == 0) {
				printf("\n");
			}
		}

    }
    void dump2() const {
    	dump2(forbiddenValuePositions);
    }
    static void dump2(const bm128* what) {
		printf("http://www.dailysudoku.com/sudoku/play.shtml?p=");
		for(int c = 0; c < 81; c++) {
			for(int d = 0; d < 9; d++) {
				if(!what[d].isBitSet(c)) {
					printf("%d", d + 1);
				}
			}
			printf(":");
		}
		printf("\n");
		fflush(NULL);
    }
    void dump3() const { //729 symbols/line
    	dump3(forbiddenValuePositions);
    }
    static void dump3(const bm128* what) {
		for(int c = 0; c < 81; c++) {
			for(int d = 0; d < 9; d++) {
				if(!what[d].isBitSet(c)) {
					printf("%d", d + 1);
				}
				else {
					printf(".");
				}
			}
		}
		printf("\n");
		fflush(NULL);
    }
    bool fromChars2(const char *src) {
		for(int d = 0; d < 9; d++) {
			forbiddenValuePositions[d] = constraints::mask81; //all forbidden
			fixedValuePositions[d].clear();
		}
    	const char* x = src;
    	const char* theEnd = src + 1000;
    	for(; x < theEnd && *x != '='; x++); //skip up to '='
    	if(x >= theEnd) return false;
    	for(int c = 0; c < 81; c++) {
    		x++; //skip separator '=' or ':'
        	for(; x < theEnd && *x != ':'; x++) {
            	if(x >= theEnd) return false;
            	int d = (*x) - '0' - 1;
            	if(d < 0 || d > 8) return false;
        		forbiddenValuePositions[d].clearBit(c); //mark as allowed
        	}
    	}
    	return true;
    }
};

struct minimizer {
	void minimizeVanilla(char *puz);
	void minimizePencilmarks(char *puz); //expand the pencilmarks for single-solution puzzle
	void minimizePencilmarks(bm128 *puz); //expand the pencilmarks for single-solution puzzle
	void reduceM2P1(bm128 *puz); //reduce forbidden placements for single-solution minimized puzzle
	void reduceM2P1(const char* p); //string as input
	void transformM1P1(bm128 *forbiddenValuePositions); //transform single-solution puzzle
	void transformM1P1(const char* p); //string as input
	void solRowMinLex(const bm128 *src, bm128 *res, const char* sol);
	bool solRowMinLex(const bm128 *src, bm128 *res); //transform single-solution puzzle to row-min-lex by solution grid
	void solRowMinLex(const bm128 *src); //transform single-solution puzzle to row-min-lex by solution grid
	void solRowMinLex(const char *p); //string as input
	void solRowMinLex(const bm128 *src, const char* sol);
};
