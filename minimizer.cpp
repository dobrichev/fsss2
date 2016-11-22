#if 1
#include <vector>
#include <algorithm>
#include <stdio.h>
#include "minimizer.h"
#include "fsss2.h"

void minimizer::minimizeVanilla(char *puz) {

	std::vector<knownNonRedundantGivens> prevPass;
	std::vector<knownNonRedundantGivens> curPass;
	uint32_t numGivens = 0; //number of givens
	uint32_t cellPos[81]; //indexes of the cells to iterate
	prevPass.reserve(65000);
	curPass.reserve(65000);

	//compose a list of givens, assume they are < 64
	for(int i = 0; i < 81; i++) {
		if(puz[i]) {
			cellPos[numGivens++] = i;
		}
	}
	if(numGivens > 64)
		return; //silently ignore the large puzzles

	knownNonRedundantGivens original;
	original.aliveGivensMask = (((uint64_t)1) << numGivens) - 1; //rightmost bumGivens bits set to 1
	original.knownNonRedundantsMask = 0;

	{
		hasSingleSolution ss;
		if(1 != ss.solve(puz))
			return; //silently ignore invalid or multiple-solution puzzles
	}

	prevPass.push_back(original); //no eliminations and no known non-redundant clues so far

	isRedundant redundancyTester;

	do { //while the previous pass returns puzzles do a next pass
		//produce puzzles a) with one less given than in previous pass; b) having unique solution; c) not necessarily minimal
		for(int currGiven = numGivens - 1; currGiven >= 0; currGiven--) { //start from most significant bit in the givens
			//uint64_t currGivenBit = (uint64_t)1 << currGiven;
			//for each subgrid from the previous pass find all possible subgrids still having unique solution
			for(std::vector<knownNonRedundantGivens>::iterator parent = prevPass.begin(); parent != prevPass.end(); parent++) {
				//if((parent->aliveGivensMask & currGivenBit) && !(parent->knownNonRedundantsMask & currGivenBit)) { //not already removed and not marked as "don't remove"
				if(parent->isForRemoval(currGiven)) { //not already removed and not marked as "don't remove"
					//uint64_t givens = parent->aliveGivensMask & (~currGivenBit); //clear the current bit
					//check whether the same givens are previously processed in the same pass
					knownNonRedundantGivens current;
					//current.aliveGivensMask = givens;
					current.getReducedGivensFrom(*parent, currGiven);
					if(std::binary_search(curPass.begin(), curPass.end(), current)) {
						//already processed
						continue;
					}
					//check whether the currGiven is redundant in this context
					{
						char p[96] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
						for(uint32_t i = 0; i < numGivens; i++) {
							if(parent->aliveGivensMask & ((uint64_t)1 << i)) {
								p[cellPos[i]] = puz[cellPos[i]];
							}
						}
						if(redundancyTester.solve(p, cellPos[currGiven])) {
							//the currGiven is redundant in the context of the parent
							current.knownNonRedundantsMask = parent->knownNonRedundantsMask;
							if(curPass.size() && current < curPass[curPass.size() - 1]) {
								//there is something wrong in the ordering
								fprintf(stderr, "********************************\n");
							}
							curPass.push_back(current); //store it to avoid duplicate processing later in this pass
							if((current.aliveGivensMask & (~current.knownNonRedundantsMask)) == 0) {
								//there are no more givens to remove => a minimal puzzle is found
								char pp[81];
								for(int i = 0; i < 81; i++) {
									pp[i] = p[i] ? p[i] + '0' : '.';
								}
								printf("%81.81s\n", pp);
							}
						}
						else {
							//parent->knownNonRedundantsMask |= currGivenBit; //mark the clue as known non-redundant in this context to avoid duplicate checking later
							parent->markAsNonRedundant(currGiven); //mark the clue as known non-redundant in this context to avoid duplicate checking later
							if((parent->aliveGivensMask & (~parent->knownNonRedundantsMask)) == 0) {
								//there are no more givens to remove => parent is proven minimal puzzle
								char pp[81];
								for(int i = 0; i < 81; i++) {
									pp[i] = p[i] ? p[i] + '0' : '.';
								}
								printf("%81.81s\n", pp);
							}
							continue; //continue with next parent
						}
					}
				}
			} //parent
		} //given
		for(int i = 0; i < (int)curPass.size() - 2; i++) {
			if(!(curPass[i] < curPass[i + 1]))	{
//				char pp[128];
				fprintf(stderr, "********************************\n");
//				for(int d = 0; d < 64; d++) {
//					curPass[i].aliveConstrainsMask[d].toMask128(pp);
//					printf("%128.128s|", pp);
//				}
//				printf("\n");
//				for(int d = 0; d < 9; d++) {
//					curPass[i + 1].aliveConstrainsMask[d].toMask128(pp);
//					printf("%128.128s|", pp);
//				}
//				printf("\n");
			}
//			else {
//				fprintf(stderr, "++\n");
//			}
		}
		prevPass.clear();
		prevPass.swap(curPass);
	} while(!prevPass.empty());
}
void minimizer::minimizePencilmarks(char *puz) {
	{
		hasSingleSolution ss;
		if(1 != ss.solve(puz))
			return; //silently ignore invalid or multiple-solution puzzles
	}
	std::vector<knownNonRedundantConstrains> prevPass;
	std::vector<knownNonRedundantConstrains> curPass;
	prevPass.reserve(65000);
	curPass.reserve(65000);

	knownNonRedundantConstrains original;
	for(int d = 0; d < 9; d++) {
		original.aliveConstrainsMask[d].clear();
		original.knownNonRedundantsMask[d].clear();
	}
	for(int c = 0; c < 81; c++) {
		if(puz[c] == 0) continue;
		for(int d = 0; d < 9; d++) {
			if(puz[c] == d) {
				original.knownNonRedundantsMask[d].setBit(c); //don't try to remove it
			}
			else {
				original.aliveConstrainsMask[d].setBit(c); //mark as "disallowed"
			}
		}
	}
	prevPass.push_back(original); //all initial constrains, known non-redundant constrains only for initial givens that must survive for the solution

	isRedundant redundancyTester;

	do { //while the previous pass returns puzzles do a next pass
		//produce puzzles a) with one less given than in previous pass; b) having unique solution; c) not necessarily minimal
		printf("Parent list is of size %d\n", (int)prevPass.size());
		for(int currDigit = 8; currDigit >= 0; currDigit--) { //iterate digits
			for(int currCell = 80; currCell >=0; currCell--) { //iterate cells in reverse order
				if(!original.isForRemoval(currDigit, currCell))
					continue; //we wouldn't find this position within the parents' list
				for(std::vector<knownNonRedundantConstrains>::iterator parent = prevPass.begin(); parent != prevPass.end(); parent++) {
					//if((parent->aliveGivensMask & currGivenBit) && !(parent->knownNonRedundantsMask & currGivenBit)) { //not already removed and not marked as "don't remove"
					if(parent->isForRemoval(currDigit, currCell)) { //not already removed and not marked as "don't remove"
						//check whether the constrains combination is previously processed in the same pass
						knownNonRedundantConstrains current;
						current.getReducedGivensFrom(*parent, currDigit, currCell);
						if(std::binary_search(curPass.begin(), curPass.end(), current)) {
							//already processed
							continue;
						}
//						{
//							char pp[88];
//							printf("********************************\n");
//							for(int i = 0; i < 9; i++) {
//								current.aliveConstrainsMask[i].toMask81(pp);
//								printf("%81.81s\n", pp);
//							}
//							printf("\n");
//							for(int i = 0; i < 9; i++) {
//								current.knownNonRedundantsMask[i].toMask81(pp);
//								printf("%81.81s\n", pp);
//							}
//						}
						//check whether the currGiven is redundant in this context
						{
							if(redundancyTester.solve(current.aliveConstrainsMask, currDigit, currCell)) {
								//the currGiven is redundant in the context of the parent
								current.getKnownNonRedundantsFrom(*parent);
//								if(curPass.size() && current < curPass[curPass.size() - 1]) {
//									//there is something wrong in the ordering
//									char pp[128];
//									fprintf(stderr, "********************************\n");
//									for(int d = 0; d < 9; d++) {
//										current.aliveConstrainsMask[d].toMask128(pp);
//										fprintf(stderr, "%128.128s|", pp);
//									}
//									fprintf(stderr, "\n");
//									for(int d = 0; d < 9; d++) {
//										curPass[curPass.size() - 1].aliveConstrainsMask[d].toMask128(pp);
//										fprintf(stderr, "%128.128s|", pp);
//									}
//									fprintf(stderr, "\n");
//								}
//								else {
//									fprintf(stderr, "+++\n");
//								}
								curPass.push_back(current); //store it to avoid duplicate processing later in this pass
								if(current.hasNothingForRemoval()) {
									//there are no more givens to remove => a minimal puzzle is found
//									char pp[81];
//									for(int i = 0; i < 81; i++) {
//										pp[i] = p[i] ? p[i] + '0' : '.';
//									}
//									printf("%81.81s\n", pp);
									printf(".");
								}
							}
							else {
								parent->markAsNonRedundant(currDigit, currCell); //mark the clue as known non-redundant in this context to avoid duplicate checking later
								if(current.hasNothingForRemoval()) {
									//there are no more givens to remove => parent is proven minimal puzzle
//									char pp[81];
//									for(int i = 0; i < 81; i++) {
//										pp[i] = p[i] ? p[i] + '0' : '.';
//									}
//									printf("%81.81s\n", pp);
									printf(",");
								}
								continue; //continue with next parent
							}
						}
					}
				} //parent
			} //cell
		} //digit
		prevPass.clear();
		prevPass.swap(curPass);
	} while(!prevPass.empty());
}
void minimizer::minimizePencilmarks(bm128 *puz) {
	char sol[88];
	{
		getSingleSolution ss;
		if(1 != ss.solve(puz, sol))
			return; //silently ignore invalid or multiple-solution puzzles
	}
	std::vector<knownNonRedundantConstrains> prevPass;
	std::vector<knownNonRedundantConstrains> curPass;
	prevPass.reserve(65000);
	curPass.reserve(65000);

	knownNonRedundantConstrains original;
	for(int d = 0; d < 9; d++) {
		original.aliveConstrainsMask[d].clear();
		original.knownNonRedundantsMask[d].clear();
	}
	for(int c = 0; c < 81; c++) {
		for(int d = 0; d < 9; d++) {
			if(sol[c] != d) {
				original.aliveConstrainsMask[d].setBit(c); //mark as "disallowed"
			}
		}
	}
	prevPass.push_back(original); //all initial constrains, known non-redundant constrains only for initial givens that must survive for the solution

	isRedundant redundancyTester;

	do { //while the previous pass returns puzzles do a next pass
		//produce puzzles a) with one less given than in previous pass; b) having unique solution; c) not necessarily minimal
		printf("Parent list is of size %d\n", (int)prevPass.size());
		for(int currDigit = 0; currDigit < 9; currDigit++) { //iterate digits
			for(int currCell = 80; currCell >=0; currCell--) { //iterate cells in reverse order
				if(!original.isForRemoval(currDigit, currCell))
					continue; //we wouldn't find this position within the parents' list
				for(std::vector<knownNonRedundantConstrains>::iterator parent = prevPass.begin(); parent != prevPass.end(); parent++) {
					//if((parent->aliveGivensMask & currGivenBit) && !(parent->knownNonRedundantsMask & currGivenBit)) { //not already removed and not marked as "don't remove"
					if(parent->isForRemoval(currDigit, currCell)) { //not already removed and not marked as "don't remove"
						//check whether the constrains combination is previously processed in the same pass
						knownNonRedundantConstrains current;
						current.getReducedGivensFrom(*parent, currDigit, currCell);
						if(std::binary_search(curPass.begin(), curPass.end(), current)) {
							//already processed
							continue;
						}
//						{
//							char pp[88];
//							printf("********************************\n");
//							for(int i = 0; i < 9; i++) {
//								current.aliveConstrainsMask[i].toMask81(pp);
//								printf("%81.81s\n", pp);
//							}
//							printf("\n");
//							for(int i = 0; i < 9; i++) {
//								current.knownNonRedundantsMask[i].toMask81(pp);
//								printf("%81.81s\n", pp);
//							}
//						}
						//check whether the currGiven is redundant in this context
						{
							if(redundancyTester.solve(current.aliveConstrainsMask, currDigit, currCell)) {
								//the currGiven is redundant in the context of the parent
								current.getKnownNonRedundantsFrom(*parent);
								curPass.push_back(current); //store it to avoid duplicate processing later in this pass
								if(current.hasNothingForRemoval()) {
									//there are no more givens to remove => a minimal puzzle is found
//									char pp[81];
//									for(int i = 0; i < 81; i++) {
//										pp[i] = p[i] ? p[i] + '0' : '.';
//									}
//									printf("%81.81s\n", pp);
									printf(".");
								}
							}
							else {
								parent->markAsNonRedundant(currDigit, currCell); //mark the clue as known non-redundant in this context to avoid duplicate checking later
								if(current.hasNothingForRemoval()) {
									//there are no more givens to remove => parent is proven minimal puzzle
//									char pp[81];
//									for(int i = 0; i < 81; i++) {
//										pp[i] = p[i] ? p[i] + '0' : '.';
//									}
//									printf("%81.81s\n", pp);
									printf(",");
								}
								continue; //continue with next parent
							}
						}
					}
				} //parent
			} //cell
		} //digit
		prevPass.clear();
		prevPass.swap(curPass);
	} while(!prevPass.empty());
}
#endif
