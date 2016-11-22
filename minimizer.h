#include <inttypes.h>
#include <memory.h>
#include "t_128.h"

struct knownNonRedundantGivens {
    uint64_t aliveGivensMask;
    uint64_t knownNonRedundantsMask;
    bool operator< (const knownNonRedundantGivens& givensMask) const {return aliveGivensMask < givensMask.aliveGivensMask;}
    void markAsNonRedundant(int pos) {knownNonRedundantsMask |= ((uint64_t)1 << pos);}
    bool isForRemoval(int pos) const {return (aliveGivensMask & ((uint64_t)1 << pos)) && !(knownNonRedundantsMask & ((uint64_t)1 << pos));}
    void getReducedGivensFrom(const knownNonRedundantGivens &src, int pos) {aliveGivensMask = src.aliveGivensMask & (~((uint64_t)1 << pos));}
};
struct knownNonRedundantConstrains {
    bm128 aliveConstrainsMask[9];
    bm128 knownNonRedundantsMask[9];
    bool operator< (const knownNonRedundantConstrains& constrainsMask) const {
    	//return memcmp(aliveConstrainsMask, constrainsMask.aliveConstrainsMask, sizeof(aliveConstrainsMask) * sizeof(aliveConstrainsMask[0])) > 0;
    	for(int g = 8; g >= 0; g--) {
			if(aliveConstrainsMask[g].bitmap128.m128i_u64[1] < constrainsMask.aliveConstrainsMask[g].bitmap128.m128i_u64[1]) return true;
			if(aliveConstrainsMask[g].bitmap128.m128i_u64[1] > constrainsMask.aliveConstrainsMask[g].bitmap128.m128i_u64[1]) return false;
			if(aliveConstrainsMask[g].bitmap128.m128i_u64[0] < constrainsMask.aliveConstrainsMask[g].bitmap128.m128i_u64[0]) return true;
			if(aliveConstrainsMask[g].bitmap128.m128i_u64[0] > constrainsMask.aliveConstrainsMask[g].bitmap128.m128i_u64[0]) return false;
    	}
    	return false;
   }
    void markAsNonRedundant(int given, int cell) {knownNonRedundantsMask[given].setBit(cell);}
    bool isForRemoval(int given, int cell) const {return (aliveConstrainsMask[given].isBitSet(cell) & !knownNonRedundantsMask[given].isBitSet(cell));}
    void getReducedGivensFrom(const knownNonRedundantConstrains &src, int given, int cell) {
    	aliveConstrainsMask[0] = src.aliveConstrainsMask[0];
    	aliveConstrainsMask[1] = src.aliveConstrainsMask[1];
    	aliveConstrainsMask[2] = src.aliveConstrainsMask[2];
    	aliveConstrainsMask[3] = src.aliveConstrainsMask[3];
    	aliveConstrainsMask[4] = src.aliveConstrainsMask[4];
    	aliveConstrainsMask[5] = src.aliveConstrainsMask[5];
    	aliveConstrainsMask[6] = src.aliveConstrainsMask[6];
    	aliveConstrainsMask[7] = src.aliveConstrainsMask[7];
    	aliveConstrainsMask[8] = src.aliveConstrainsMask[8];
    	aliveConstrainsMask[given].clearBit(cell);
    }
    void getKnownNonRedundantsFrom(const knownNonRedundantConstrains &src) {
    	knownNonRedundantsMask[0] = src.knownNonRedundantsMask[0];
    	knownNonRedundantsMask[1] = src.knownNonRedundantsMask[1];
    	knownNonRedundantsMask[2] = src.knownNonRedundantsMask[2];
    	knownNonRedundantsMask[3] = src.knownNonRedundantsMask[3];
    	knownNonRedundantsMask[4] = src.knownNonRedundantsMask[4];
    	knownNonRedundantsMask[5] = src.knownNonRedundantsMask[5];
    	knownNonRedundantsMask[6] = src.knownNonRedundantsMask[6];
    	knownNonRedundantsMask[7] = src.knownNonRedundantsMask[7];
    	knownNonRedundantsMask[8] = src.knownNonRedundantsMask[8];
    }
    bool hasNothingForRemoval() const {
		//if((current.aliveGivensMask & (~current.knownNonRedundantsMask)) == 0)
			//there are no more givens to remove => a minimal puzzle is found
    	return
    			aliveConstrainsMask[0].isSubsetOf(knownNonRedundantsMask[0]) &&
    			aliveConstrainsMask[1].isSubsetOf(knownNonRedundantsMask[1]) &&
    			aliveConstrainsMask[2].isSubsetOf(knownNonRedundantsMask[2]) &&
    			aliveConstrainsMask[3].isSubsetOf(knownNonRedundantsMask[3]) &&
    			aliveConstrainsMask[4].isSubsetOf(knownNonRedundantsMask[4]) &&
    			aliveConstrainsMask[5].isSubsetOf(knownNonRedundantsMask[5]) &&
    			aliveConstrainsMask[6].isSubsetOf(knownNonRedundantsMask[6]) &&
    			aliveConstrainsMask[7].isSubsetOf(knownNonRedundantsMask[7]) &&
    			aliveConstrainsMask[8].isSubsetOf(knownNonRedundantsMask[8]);
    }
};

struct minimizer {
	void minimizeVanilla(char *puz);
	void minimizePencilmarks(char *puz);
	void minimizePencilmarks(bm128 *puz);
};
