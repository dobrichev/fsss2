#if 1
#include <vector>
#include <set>
#include <algorithm>
#include <stdio.h>
#include <random>
#include <experimental/algorithm> //gcc-specific
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
	auto rg(std::mt19937{std::random_device{}()});
	//auto rg(std::mt19937{});
	//auto rg(std::mt19937{0});
	size_t numMinimals = 0;
	int numPencilmarks = 81 * 9;
	int numPasses = 0;
	char sol[256];
//	{
//		hasSingleSolution ss;
//		if(1 != ss.solve(puz))
//			return; //silently ignore invalid or multiple-solution puzzles
//	}
	{
		getSingleSolution ss;
		int nSol = ss.solve(puz, sol);
		if(1 != nSol) {
			printf("%d,", nSol);
			return; //silently ignore invalid or multiple-solution puzzles
		}
	}

//	std::vector<complementaryPencilmarksX> prevPass;
//	std::vector<complementaryPencilmarksX> curPass;
//	prevPass.reserve(65000);
//	curPass.reserve(65000);

	std::set<complementaryPencilmarksX> previousPass;
	std::set<complementaryPencilmarksX> currentPass;

	complementaryPencilmarksX original;
	for(int d = 0; d < 9; d++) {
		original.forbiddenValuePositions[d].clear(); //allowed on all positions
		original.fixedValuePositions[d].clear(); //any position could be disallowed
	}
	for(int c = 0; c < 81; c++) {
		if(puz[c] == 0) continue;
		for(int d = 0; d < 9; d++) {
			if((puz[c] - 1) == d) {
				original.fixedValuePositions[d].setBit(c); //don't disallow the target solution
			}
			else {
				original.forbiddenValuePositions[d].setBit(c); //initially all values except the given are disallowed in this cell
			}
		}
		numPencilmarks -= 8;
	}
	//prevPass.push_back(original); //all initial constraints, known non-redundant constrains only for initial givens that must survive for the solution
	previousPass.insert(original); //all initial constraints, known non-redundant constrains only for initial givens that must survive for the solution

	isRedundant redundancyTester;

	//original.dump();

	do { //while the previous pass returns puzzles do a next pass
		//produce puzzles a) with one less given than in previous pass; b) having unique solution; c) not necessarily minimal
		printf("Pass %d, maximizing %d pencilmarks. Parent list is of size\t%d\n", numPasses++, numPencilmarks++, (int)previousPass.size());
		for(int currDigit = 8; currDigit >= 0; currDigit--) { //iterate digits
			for(int currCell = 80; currCell >=0; currCell--) { //iterate cells in reverse order
				if(!original.isForRemoval(currDigit, currCell))
					continue; //we wouldn't find this position within the parents' list
				//for(auto parent : prevPass) {
				for(auto& parent : previousPass) {
					//if((parent->aliveGivensMask & currGivenBit) && !(parent->knownNonRedundantsMask & currGivenBit)) { //not already removed and not marked as "don't remove"
					if(parent.isForRemoval(currDigit, currCell)) { //not already removed and not marked as "don't remove"
						//check whether the constraints combination is previously processed in the same pass
						complementaryPencilmarksX current;
						current.getReducedForbiddensFrom(parent, currDigit, currCell); //copy parent, then allow currDigit on currCell
						//if(std::binary_search(curPass.begin(), curPass.end(), current)) {
//						if(currentPass.find(current) != currentPass.end()) {
//							//already processed
//							//printf("f");
//							continue;
//						}
						//check whether the currGiven is redundant in this context
						{
							if(redundancyTester.solve(current.forbiddenValuePositions, currDigit, currCell)) {
								//the currGiven is redundant in the context of the parent
								current.getFixedFrom(parent);
								current.markAsFixed(currDigit, currCell);
								//curPass.push_back(current); //store it to avoid duplicate processing later in this pass
								currentPass.insert(current); //store it to avoid duplicate processing later in this pass
								if(current.isMinimal()) {
									//there are no more givens to remove => current is a minimal puzzle
									printf("c");
									numMinimals++;
									if(!current.isMinimalUniqueDoubleCheck(sol)) return;
									printf("\n");
									//current.dump1(current.forbiddenValuePositions, true);
									current.dump2();
								}
								//if(curPass.size() >= 1000) goto next_pass; //memory overflow
							}
							else {
								const_cast<complementaryPencilmarksX&>(parent).markAsFixed(currDigit, currCell); //mark the clue as known non-redundant in this context to avoid duplicate checking later
								if(parent.isMinimal()) {
									//there are no more givens to remove => parent is proven minimal puzzle
									printf("p");
									numMinimals++;
									if(!parent.isMinimalUniqueDoubleCheck(sol)) return;
									printf("\n");
									//parent.dump1(parent.forbiddenValuePositions, true);
									parent.dump2();
								}
								continue; //continue with next parent
							}
						}
					}
					else {
						//printf("-");
					}
				} //parent
			} //cell
		} //digit
		//next_pass:;
		previousPass.clear();
		//std::experimental::sample(std::begin(currentPass), std::end(currentPass), std::inserter(previousPass, previousPass.end()), 5, std::mt19937{/*std::random_device{}()*/}); //gcc-specific
		std::experimental::sample(std::begin(currentPass), std::end(currentPass), std::inserter(previousPass, previousPass.end()), 10, rg); //gcc-specific
		currentPass.clear();
//		//prevPass.clear();
//		//prevPass.swap(curPass); //move all (= overflow)
//		{ //random sample
//			// NOTE: at this point we are throwing away some possibly minimized (but not identified as minimal) puzzles from curPass!
//			//prevPass.swap(curPass);
//			//std::experimental::sample(std::begin(curPass), std::end(curPass), std::back_inserter(prevPass), 10, std::mt19937{std::random_device{}()}); //gcc-specific
//			//if(!std::is_sorted(std::begin(curPass), std::end(curPass))) { //debug
//			//	printf("\ncurPass not sorted\n");
//			//}
//			std::experimental::sample(std::begin(curPass), std::end(curPass), std::back_inserter(prevPass), 10, std::mt19937{std::random_device{}()}); //gcc-specific
//			curPass.clear();
//			if(!std::is_sorted(std::begin(prevPass), std::end(prevPass))) { //debug
//				std::sort(std::begin(prevPass), std::end(prevPass));
//				//printf("\nnot sorted\n");
//				auto last = std::unique(prevPass.begin(), prevPass.end()); //debug
//				if(last != prevPass.end()) { //debug
//					prevPass.erase(last, prevPass.end());
//					printf("\nnot unique\n");
//				}
//			}
//		}
		fflush(NULL);
	} while(!previousPass.empty());
	printf("\nMinimals found = %lu\n", numMinimals);
}
void minimizer::minimizePencilmarks(bm128 *forbiddenValuePositions) {
	char sol[88];
	{
		getSingleSolution ss;
		if(1 != ss.solve(forbiddenValuePositions, sol))
			return; //silently ignore invalid or multiple-solution puzzles
	}
	std::vector<complementaryPencilmarksX> prevPass;
	std::vector<complementaryPencilmarksX> curPass;
	prevPass.reserve(65000);
	curPass.reserve(65000);

	complementaryPencilmarksX original;
	for(int d = 0; d < 9; d++) {
		original.forbiddenValuePositions[d].clear();
		original.fixedValuePositions[d].clear();
	}
	for(int c = 0; c < 81; c++) {
		for(int d = 0; d < 9; d++) {
			if(sol[c] != d) {
				original.forbiddenValuePositions[d].setBit(c); //mark as "disallowed"
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
				for(std::vector<complementaryPencilmarksX>::iterator parent = prevPass.begin(); parent != prevPass.end(); parent++) {
					//if((parent->aliveGivensMask & currGivenBit) && !(parent->knownNonRedundantsMask & currGivenBit)) { //not already removed and not marked as "don't remove"
					if(parent->isForRemoval(currDigit, currCell)) { //not already removed and not marked as "don't remove"
						//check whether the constrains combination is previously processed in the same pass
						complementaryPencilmarksX current;
						current.getReducedForbiddensFrom(*parent, currDigit, currCell);
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
							if(redundancyTester.solve(current.forbiddenValuePositions, currDigit, currCell)) {
								//the currGiven is redundant in the context of the parent
								current.getFixedFrom(*parent);
								curPass.push_back(current); //store it to avoid duplicate processing later in this pass
								if(current.isMinimal()) {
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
								parent->markAsFixed(currDigit, currCell); //mark the clue as known non-redundant in this context to avoid duplicate checking later
								if(current.isMinimal()) {
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
void minimizer::reduceM2P1(const char* p) {
	complementaryPencilmarksX src;
	if(!src.fromChars2(p)) return; //silently ignore invalid inputs
	reduceM2P1(src.forbiddenValuePositions); //do the job
}
//void minimizer::reduceM2P1(bm128 *forbiddenValuePositions) {
//	multiSolutionPM ms;
//	hasSingleSolution ss;
//	bm128 res[9];
//	//apply {-2} and get multiple-solution puzzle
//	for(int d1 = 0; d1 < 9; d1++) {
//		for(int c1 = 0; c1 < 81; c1++) {
//			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
//			forbiddenValuePositions[d1].clearBit(c1); //allow
//			for(int d2 = d1; d2 < 9; d2++) {
//				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
//					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
//					forbiddenValuePositions[d2].clearBit(c2); //allow
//					if(ms.solve(forbiddenValuePositions, res, 640)) { //iterate all solutions and collect the union of the solutions
//						//does a bi-value cell exist?
//						bm128 duplicates;
//						{
//							duplicates.clear();
//							bm128 all = duplicates;
//							bm128 triplicates = all;
//							for(int d = 0; d < 9; d++) {
//								bm128 tmp = res[d];
//								tmp &= all; //at least once found before
//								bm128 tmp2 = tmp;
//								tmp2 &= duplicates; //at least twice found before
//								triplicates |= tmp2;
//								duplicates |= tmp;
//								all |= res[d];
//							}
//							duplicates.clearBits(triplicates); //clear positions having 3+ candidates
//						}
//						if(!duplicates.isZero()) {
//							int c;
//							while(-1 != (c = duplicates.getFirstBit1Index96())) {
//								//forbidding one of 2 possibilities for cell c would resolve the cell, but not necessarily the whole solution
//								int valuesProcessed = 0;
//								for(int d = 0; d < 9 && valuesProcessed < 2; d++) {
//									if(!(res[d].isBitSet(c))) continue;
//									forbiddenValuePositions[d].setBit(c); // forbid d in c
//									if(1 == ss.solve(forbiddenValuePositions)) {
//										//lucky
//										complementaryPencilmarksX::dump2(forbiddenValuePositions);
//									}
//									forbiddenValuePositions[d].clearBit(c); // restore
//									valuesProcessed++;
//								}
//								duplicates.clearBit(c);
//							}
//						}
//					}
//					forbiddenValuePositions[d2].setBit(c2); //restore
//				} //c2
//			} //d2
//			forbiddenValuePositions[d1].setBit(c1); //restore
//		} //c1
//	} //d1
//}
//void minimizer::reduceM2P1(bm128 *forbiddenValuePositions) {
//	hasSingleSolution ss;
//	//apply {-2} and get multiple-solution puzzle
//	for(int d1 = 0; d1 < 9; d1++) {
//		for(int c1 = 0; c1 < 81; c1++) {
//			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
//			forbiddenValuePositions[d1].clearBit(c1); //allow
//			for(int d2 = d1; d2 < 9; d2++) {
//				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
//					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
//					forbiddenValuePositions[d2].clearBit(c2); //allow
//					//apply {+1} and check if single-solution is found
//					for(int d = 0; d < 9; d++) {
//						for(int c = 0; c < 81; c++) {
//							if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
//							if(d == d1 && c == c1) continue;
//							if(d == d2 && c == c2) continue;
//							forbiddenValuePositions[d].setBit(c); // forbid d in c
//							if(1 == ss.solve(forbiddenValuePositions)) {
//								//lucky
//								complementaryPencilmarksX::dump2(forbiddenValuePositions);
//							}
//							forbiddenValuePositions[d].clearBit(c); // restore
//						}
//					}
//					forbiddenValuePositions[d2].setBit(c2); //restore
//				} //c2
//			} //d2
//			forbiddenValuePositions[d1].setBit(c1); //restore
//		} //c1
//	} //d1
//}
//void minimizer::reduceM2P1(bm128 *forbiddenValuePositions) {
//	multiSolutionPM ms;
//	bm128 blackList[9][81];
//	bm128 res[9];
//	hasSingleSolution ss;
//	//for each {-1} compose a blacklist of placements that require more than one additional forbidden to solve
//	for(int d1 = 0; d1 < 9; d1++) {
//		for(int c1 = 0; c1 < 81; c1++) {
//			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
//			forbiddenValuePositions[d1].clearBit(c1); //allow
//			ms.solve(forbiddenValuePositions, res, 10000); //iterate some solutions and collect the union of the solutions
//			//blacklist three-value cells
//			bm128 triplicates;
//			{
//				triplicates.clear();
//				bm128 all = triplicates;
//				bm128 duplicates = triplicates;
//				for(int d = 0; d < 9; d++) {
//					bm128 tmp = res[d];
//					tmp &= all; //at least once found before
//					bm128 tmp2 = tmp;
//					tmp2 &= duplicates; //at least twice found before
//					triplicates |= tmp2;
//					duplicates |= tmp;
//					all |= res[d];
//				}
//			}
//			//this {-1} alone requires {+2} for the respective cell. Later skip doing {+1} for the cells set, for either [d1][c1] and [d2][c2]
//			blackList[d1][c1] = triplicates;
//			forbiddenValuePositions[d1].setBit(c1); //restore
//		} //c1
//	} //d1
//	//apply {-2} and get multiple-solution puzzle
//	for(int d1 = 0; d1 < 9; d1++) {
//		for(int c1 = 0; c1 < 81; c1++) {
//			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
//			forbiddenValuePositions[d1].clearBit(c1); //allow
//			for(int d2 = d1; d2 < 9; d2++) {
//				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
//					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
//					forbiddenValuePositions[d2].clearBit(c2); //allow
//					//apply {+1} and check if single-solution is found
//					for(int d = 0; d < 9; d++) {
//						for(int c = 0; c < 81; c++) {
//							if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
//							if(d == d1 && c == c1) continue;
//							if(d == d2 && c == c2) continue;
//							if(blackList[d1][c1].isBitSet(c)) continue; //requires 2+ more constraints
//							if(blackList[d2][c2].isBitSet(c)) continue; //requires 2+ more constraints
//							forbiddenValuePositions[d].setBit(c); // forbid d in c
//							if(1 == ss.solve(forbiddenValuePositions)) {
//								//lucky
//								complementaryPencilmarksX::dump2(forbiddenValuePositions);
//							}
//							forbiddenValuePositions[d].clearBit(c); // restore
//						}
//					}
//					forbiddenValuePositions[d2].setBit(c2); //restore
//				} //c2
//			} //d2
//			forbiddenValuePositions[d1].setBit(c1); //restore
//		} //c1
//	} //d1
//}
void minimizer::reduceM2P1(bm128 *forbiddenValuePositions) {
	bm128 blackList[9][81][9];
	hasSingleSolution ss;
	//for {-{x,y},+z} success both {-x,+z} and {-y,+z} must success
	//for each {-1} compose a blacklist
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//apply {+1} and check if single-solution is found
			for(int d = 0; d < 9; d++) {
				blackList[d1][c1][d].clear();
				for(int c = 0; c < 81; c++) {
					if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
					if(d == d1 && c == c1) continue;
					forbiddenValuePositions[d].setBit(c); // forbid d in c
					if(2 == ss.solve(forbiddenValuePositions)) {
						blackList[d1][c1][d].setBit(c); //multiple-solution => blacklisted
					}
					forbiddenValuePositions[d].clearBit(c); // restore
				}
			}
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	//apply {-2} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			for(int d2 = d1; d2 < 9; d2++) {
				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
					forbiddenValuePositions[d2].clearBit(c2); //allow
					//apply {+1} and check if single-solution is found
					for(int d = 0; d < 9; d++) {
						for(int c = 0; c < 81; c++) {
							if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
							if(d == d1 && c == c1) continue;
							if(d == d2 && c == c2) continue;
							if(blackList[d1][c1][d].isBitSet(c)) continue; //requires 2+ more constraints
							if(blackList[d2][c2][d].isBitSet(c)) continue; //requires 2+ more constraints
							forbiddenValuePositions[d].setBit(c); // forbid d in c
							if(1 == ss.solve(forbiddenValuePositions)) {
								//lucky
								complementaryPencilmarksX::dump2(forbiddenValuePositions);
							}
							forbiddenValuePositions[d].clearBit(c); // restore
						}
					}
					forbiddenValuePositions[d2].setBit(c2); //restore
				} //c2
			} //d2
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
}
#endif
