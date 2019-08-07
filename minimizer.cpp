#if 1
#include <vector>
#include <set>
#include <list>
#include <map>
#include <algorithm>
#include <stdio.h>
#include <random>
#include <experimental/algorithm> //gcc-specific
#include "minimizer.h"
#include "rowminlex.h"
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
		std::experimental::sample(std::begin(currentPass), std::end(currentPass), std::inserter(previousPass, previousPass.end()), 100, rg); //gcc-specific
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
void minimizer::minimizePencilmarks(pencilmarks& forbiddenValuePositions) {
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
	//reduceM2P1(src.forbiddenValuePositions); //do the job
	//reduceM2P1v2(src.forbiddenValuePositions); //do the job
	//reduceM2P1v3(src.forbiddenValuePositions); //do the job
	reduceM2P1v4(src.forbiddenValuePositions); //do the job
}
void minimizer::transformM1P1(const char* p) {
	complementaryPencilmarksX src;
	if(!src.fromChars2(p)) return; //silently ignore invalid inputs
	transformM1P1(src.forbiddenValuePositions); //do the job
}
void minimizer::transformM2P2(const char* p) {
	complementaryPencilmarksX src;
	if(!src.fromChars2(p)) return; //silently ignore invalid inputs
	transformM2P2(src.forbiddenValuePositions); //do the job
}
void minimizer::solRowMinLex(const char* p) {
	complementaryPencilmarksX src;
	if(!src.fromChars2(p)) return; //silently ignore invalid inputs
	solRowMinLex(src.forbiddenValuePositions); //do the job
}
void minimizer::tryReduceM1(const char* p) {
	complementaryPencilmarksX src;
	if(!src.fromChars2(p)) return; //silently ignore invalid inputs
	tryReduceM1(src.forbiddenValuePositions); //do the job
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
//void minimizer::reduceM2P1(bm128 *forbiddenValuePositions) {
//	bm128 blackList[9][81][9];
//	hasSingleSolution ss;
//	//for {-{x,y},+z} success both {-x,+z} and {-y,+z} must success
//	//for each {-1} compose a blacklist
//	for(int d1 = 0; d1 < 9; d1++) {
//		for(int c1 = 0; c1 < 81; c1++) {
//			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
//			forbiddenValuePositions[d1].clearBit(c1); //allow
//			//apply {+1} and check if single-solution is found
//			for(int d = 0; d < 9; d++) {
//				blackList[d1][c1][d].clear();
//				for(int c = 0; c < 81; c++) {
//					if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
//					if(d == d1 && c == c1) continue;
//					forbiddenValuePositions[d].setBit(c); // forbid d in c
//					if(2 == ss.solve(forbiddenValuePositions)) {
//						blackList[d1][c1][d].setBit(c); //multiple-solution => blacklisted
//					}
//					forbiddenValuePositions[d].clearBit(c); // restore
//				}
//			}
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
//							if(blackList[d1][c1][d].isBitSet(c)) continue; //requires 2+ more constraints
//							if(blackList[d2][c2][d].isBitSet(c)) continue; //requires 2+ more constraints
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

#ifdef COUNT_TRIALS
	extern int nTrials;
#endif

//	int numSolverCalls = 0;
//
//	extern int knownNoLockedCandidatesHits;
//	extern int knownNoLockedCandidatesMisses;
//	extern int knownNoHiddenHits;
//	extern int knownNoHiddenMisses;

void minimizer::reduceM2P1(pencilmarks& forbiddenValuePositions) { // ~1 second/puzzle
//	numSolverCalls = 0;
//	knownNoLockedCandidatesHits = 0;
//	knownNoLockedCandidatesMisses = 0;
//	knownNoHiddenHits = 0;
//	knownNoHiddenMisses = 0;
	fprintf(stderr, ".");
	bm128 exchangable[9][81][9];
	getSingleSolution ss;
	char sol[88]; //pass solution as hint parameter to canonicalizer
#ifdef COUNT_TRIALS
	int maxTrialsSoFar = 0;
#endif
	//compose a list of validity preserving mutually exchangable forbiddenValuePositions
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			for(int d = 0; d < 9; d++) {
				exchangable[d1][c1][d].clear();
			}
		}
	}
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//apply {+1} and check if single-solution is found
			for(int d = 0; d < 9; d++) {
				for(int c = 0; c < 81; c++) {
					if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
					if(d == d1 && c == c1) continue;
					forbiddenValuePositions[d].setBit(c); // forbid d in c
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					int numSolutions = ss.solve(forbiddenValuePositions, sol);
					//numSolverCalls++;
					if(1 == numSolutions) {
						exchangable[d][c][d1].setBit(c1); // forbidden at {d1, c1} is exchangable with {d2, c2}
						solRowMinLex(forbiddenValuePositions, sol); // export {-1,+1} NOTE: This exports non-minimals!!!
					}
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
						//fprintf(stderr, "\n%d", maxTrialsSoFar);
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),+(%d,%d)}\t%d", nTrials, d1, c1, d, c, numSolutions);
#endif
					forbiddenValuePositions[d].clearBit(c); // restore
				}
			}
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	//if union(exchangable[d][c][*]) has > 1 bit set, then all forbiddens from * can be removed together, and replaced by the single [d][c]
	//apply {-2} and get multiple-solution puzzle
	for(int dDest = 0; dDest < 9; dDest++) {
		for(int cDest = 0; cDest < 81; cDest++) {
			if(forbiddenValuePositions[dDest].isBitSet(cDest)) continue; //skip forbidden placements
			bm128 u;
			u.clear();
			int removeCount = 0;
			for(int d = 0; d < 9; d++) {
				removeCount += exchangable[dDest][cDest][d].popcount_128();
				u |= exchangable[dDest][cDest][d];
			}
			if(removeCount < 2) continue; // no reduction possible
			//if(removeCount > 2) {
			//	fprintf(stderr, "%d,", removeCount);
			//}
			forbiddenValuePositions[dDest].setBit(cDest); //forbid
			int removedCount = 0;
			std::pair<int,int> suited[9*81];
			for(int c = 0; c < 81 && removedCount < removeCount; c++) {
				if(!u.isBitSet(c)) continue;
				for(int d = 0; d < 9 && removedCount < removeCount; d++) {
					if(!exchangable[dDest][cDest][d].isBitSet(c)) continue;
					forbiddenValuePositions[d].clearBit(c); //allow
					suited[removedCount] = std::pair<int,int>(d, c);
					removedCount++;
				}
			}
			//remove only 2 at a time in all possible ways
			for(int src1 = 0; src1 < removeCount - 1; src1++) {
				forbiddenValuePositions[suited[src1].first].clearBit(suited[src1].second); //allow
				for(int src2 = src1 + 1; src2 < removeCount; src2++) {
					forbiddenValuePositions[suited[src2].first].clearBit(suited[src2].second); //allow
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, sol);
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
						//fprintf(stderr, "\n%d", maxTrialsSoFar);
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),(%d,%d),+(%d,%d)}\t%d", nTrials, suited[src1].first, suited[src1].second, suited[src2].first, suited[src2].second, dDest, cDest, numSolutions);
#endif
					if(1 == numSolutions) {
						//lucky
						solRowMinLex(forbiddenValuePositions, sol); // export {-2,+1}
						//complementaryPencilmarksX::dump2(forbiddenValuePositions); // export {-2,+1}
						fprintf(stderr, "+");
					}
					forbiddenValuePositions[suited[src2].first].setBit(suited[src2].second); //forbid
				} //src2
				forbiddenValuePositions[suited[src1].first].setBit(suited[src1].second); //forbid
			} //src1
			forbiddenValuePositions[dDest].clearBit(cDest); //restore
		} //cDest
	} //dDest
	//fprintf(stderr, "(-+)\t%d\t%d\t%d\t%d\t%d\n", numSolverCalls, knownNoLockedCandidatesHits, knownNoLockedCandidatesMisses, knownNoHiddenHits, knownNoHiddenMisses);
}
void minimizer::reduceM2P1v2(pencilmarks& forbiddenValuePositions) { // ~1.5 seconds/puzzle
	fprintf(stderr, ".");
//	numSolverCalls = 0;
//	knownNoLockedCandidatesHits = 0;
//	knownNoLockedCandidatesMisses = 0;
//	knownNoHiddenHits = 0;
//	knownNoHiddenMisses = 0;
	getSingleSolution ss;
	struct solution {
		char sol[88];
	};
	solution sol; //pass solution as hint parameter to canonicalizer
	struct redundantInContext {
		int digit;
		int cell;
		bool skip;
		solution sol;
	};
	redundantInContext redundantsAlone[9*81]; // pair of {digit, cell} for each redundant forbidden in the context
#ifdef COUNT_TRIALS
	int maxTrialsSoFar = 0;
#endif
	//apply {+1} and check if single-solution is found
	for(int dForbid = 0; dForbid < 9; dForbid++) {
		for(int cForbid = 0; cForbid < 81; cForbid++) {
			if(forbiddenValuePositions[dForbid].isBitSet(cForbid)) continue; //skip already forbidden placements
			forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
			int numRedundantsAlone = 0;
			//apply {-1} and check if single-solution is found
			for(int dAllow = 0; dAllow < 9; dAllow++) {
				for(int cAllow = 0; cAllow < 81; cAllow++) {
					if(!forbiddenValuePositions[dAllow].isBitSet(cAllow)) continue; //skip allowed placements
					if(dForbid == dAllow && cForbid == cAllow) continue; //note that dForbid at cForbid is temporary set
					forbiddenValuePositions[dAllow].clearBit(cAllow); //allow
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, redundantsAlone[numRedundantsAlone].sol.sol);
					if(1 == numSolutions) {
						redundantsAlone[numRedundantsAlone].digit = dAllow;
						redundantsAlone[numRedundantsAlone].cell = cAllow;
						redundantsAlone[numRedundantsAlone].skip = false;
						numRedundantsAlone++;
					}
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
						//fprintf(stderr, "\n%d", maxTrialsSoFar);
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),+(%d,%d)}\t%d", nTrials, dAllow, cAllow, dForbid, cForbid, numSolutions);
#endif
					forbiddenValuePositions[dAllow].setBit(cAllow); //restore
				} //cAllow
			} //dAllow
			//apply {-2} and check if single-solution is found
			for(int allow1 = 0; allow1 < numRedundantsAlone - 1; allow1++) {
				forbiddenValuePositions[redundantsAlone[allow1].digit].clearBit(redundantsAlone[allow1].cell); //allow
				for(int allow2 = allow1 + 1; allow2 < numRedundantsAlone; allow2++) {
					forbiddenValuePositions[redundantsAlone[allow2].digit].clearBit(redundantsAlone[allow2].cell); //allow
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, sol.sol);
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),(%d,%d),+(%d,%d)}\t%d", nTrials, redundantsAlone[allow1].digit, redundantsAlone[allow1].cell, redundantsAlone[allow2].digit, redundantsAlone[allow2].cell, dForbid, cForbid, numSolutions);
#endif
					if(1 == numSolutions) {
						//lucky
						redundantsAlone[allow1].skip = true; //don't export {-1,+1} later because it has redundant
						redundantsAlone[allow2].skip = true; //don't export {-1,+1} later because it has redundant
						solRowMinLex(forbiddenValuePositions, sol.sol); // export {-2,+1}
						fprintf(stderr, "+");
					}
					forbiddenValuePositions[redundantsAlone[allow2].digit].setBit(redundantsAlone[allow2].cell); //forbid
				} //allow2
				forbiddenValuePositions[redundantsAlone[allow1].digit].setBit(redundantsAlone[allow1].cell); //forbid
			} //allow1
			//export {-1,+1}
			for(int allow1 = 0; allow1 < numRedundantsAlone; allow1++) {
				if(redundantsAlone[allow1].skip) continue; //part of {-2,+1}
				forbiddenValuePositions[redundantsAlone[allow1].digit].clearBit(redundantsAlone[allow1].cell); //allow
				solRowMinLex(forbiddenValuePositions, redundantsAlone[allow1].sol.sol); // export {-1,+1}
				forbiddenValuePositions[redundantsAlone[allow1].digit].setBit(redundantsAlone[allow1].cell); //forbid
			}
			forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
		} //cForbid
	} //dForbid
	//fprintf(stderr, "(+-)\t%d\t%d\t%d\t%d\t%d\n", numSolverCalls, knownNoLockedCandidatesHits, knownNoLockedCandidatesMisses, knownNoHiddenHits, knownNoHiddenMisses);
}
void minimizer::reduceM2P1v3(pencilmarks& forbiddenValuePositions) { // ~1.5 seconds/puzzle
	fprintf(stderr, ".");
//	numSolverCalls = 0;
//	knownNoLockedCandidatesHits = 0;
//	knownNoLockedCandidatesMisses = 0;
//	knownNoHiddenHits = 0;
//	knownNoHiddenMisses = 0;
	getSingleSolution ss;
	struct solution {
		char sol[88];
	};
	solution sol; //pass solution as hint parameter to canonicalizer
	struct redundantInContext {
		int digit;
		int cell;
		bool skip;
		solution sol;
	};
	struct redundantsInContext {
		redundantInContext rc[9*81];
		int size;
	};
	//redundantsInContext redundantsAlone[9][81]; //causes problems in stack allocation
	typedef redundantsInContext (redundantsInContext81_t)[81];
	redundantsInContext81_t* redundantsAlone = new redundantsInContext81_t[9]; //do heap allocation
#ifdef COUNT_TRIALS
	int maxTrialsSoFar = 0;
#endif
	for(int dForbid = 0; dForbid < 9; dForbid++) {
		for(int cForbid = 0; cForbid < 81; cForbid++) {
			//if(forbiddenValuePositions[dForbid].isBitSet(cForbid)) continue; //skip already forbidden placements
			redundantsAlone[dForbid][cForbid].size = 0;
		}
	}
	//pass 1: collect single-solution puzzles at {-1,+1}
	//apply {+1} and check if single-solution is found
	for(int dAllow = 0; dAllow < 9; dAllow++) {
		for(int cAllow = 0; cAllow < 81; cAllow++) {
			if(!forbiddenValuePositions[dAllow].isBitSet(cAllow)) continue; //skip allowed placements
			forbiddenValuePositions[dAllow].clearBit(cAllow); //allow
			for(int dForbid = 0; dForbid < 9; dForbid++) {
				for(int cForbid = 0; cForbid < 81; cForbid++) {
					if(forbiddenValuePositions[dForbid].isBitSet(cForbid)) continue; //skip already forbidden placements
					if(dForbid == dAllow && cForbid == cAllow) continue; //note that dForbid at cForbid is temporary set
					forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
					//apply {-1} and check if single-solution is found
#ifdef COUNT_TRIALS
							nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, redundantsAlone[dForbid][cForbid].rc[redundantsAlone[dForbid][cForbid].size].sol.sol);
					if(1 == numSolutions) {
						redundantsAlone[dForbid][cForbid].rc[redundantsAlone[dForbid][cForbid].size].digit = dAllow;
						redundantsAlone[dForbid][cForbid].rc[redundantsAlone[dForbid][cForbid].size].cell = cAllow;
						redundantsAlone[dForbid][cForbid].rc[redundantsAlone[dForbid][cForbid].size].skip = false;
						redundantsAlone[dForbid][cForbid].size++;
					}
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
						//fprintf(stderr, "\n%d", maxTrialsSoFar);
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),+(%d,%d)}\t%d", nTrials, dAllow, cAllow, dForbid, cForbid, numSolutions);
#endif
					forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
				} //cForbid
			} //dForbid
			forbiddenValuePositions[dAllow].setBit(cAllow); //restore
		} //cAllow
	} //dAllow
	//pass 2: test for {-2,+1}
	for(int dForbid = 0; dForbid < 9; dForbid++) {
		for(int cForbid = 0; cForbid < 81; cForbid++) {
			//if(redundantsAlone[dForbid][cForbid].size < 2) continue; <--- doesn't export {-1,+1}
			if(forbiddenValuePositions[dForbid].isBitSet(cForbid)) continue; //skip already forbidden placements
			forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
			//apply {-2} and check if single-solution is found
			for(int allow1 = 0; allow1 < redundantsAlone[dForbid][cForbid].size - 1; allow1++) {
				forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow1].digit].clearBit(redundantsAlone[dForbid][cForbid].rc[allow1].cell); //allow
				for(int allow2 = allow1 + 1; allow2 < redundantsAlone[dForbid][cForbid].size; allow2++) {
					forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow2].digit].clearBit(redundantsAlone[dForbid][cForbid].rc[allow2].cell); //allow
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, sol.sol);
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),(%d,%d),+(%d,%d)}\t%d", nTrials, redundantsAlone[dForbid][cForbid].rc[allow1].digit, redundantsAlone[dForbid][cForbid].rc[allow1].cell, redundantsAlone[dForbid][cForbid].rc[allow2].digit, redundantsAlone[dForbid][cForbid].rc[allow2].cell, dForbid, cForbid, numSolutions);
#endif
					if(1 == numSolutions) {
						//lucky
						redundantsAlone[dForbid][cForbid].rc[allow1].skip = true; //don't export {-1,+1} later because it has redundant
						redundantsAlone[dForbid][cForbid].rc[allow2].skip = true; //don't export {-1,+1} later because it has redundant
						solRowMinLex(forbiddenValuePositions, sol.sol); // export {-2,+1}
						fprintf(stderr, "+");
					}
					forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow2].digit].setBit(redundantsAlone[dForbid][cForbid].rc[allow2].cell); //restore
				} //allow2
				forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow1].digit].setBit(redundantsAlone[dForbid][cForbid].rc[allow1].cell); //restore
			} //allow1
			//export {-1,+1}
			for(int allow1 = 0; allow1 < redundantsAlone[dForbid][cForbid].size; allow1++) {
				if(redundantsAlone[dForbid][cForbid].rc[allow1].skip) continue; //part of {-2,+1}
				forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow1].digit].clearBit(redundantsAlone[dForbid][cForbid].rc[allow1].cell); //allow
				solRowMinLex(forbiddenValuePositions, redundantsAlone[dForbid][cForbid].rc[allow1].sol.sol); // export {-1,+1}
				forbiddenValuePositions[redundantsAlone[dForbid][cForbid].rc[allow1].digit].setBit(redundantsAlone[dForbid][cForbid].rc[allow1].cell); //restore
			}
			forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
		} //cForbid
	} //dForbid
	//fprintf(stderr, "(+-)\t%d\t%d\t%d\t%d\t%d\n", numSolverCalls, knownNoLockedCandidatesHits, knownNoLockedCandidatesMisses, knownNoHiddenHits, knownNoHiddenMisses);
	delete[] redundantsAlone;
}
void minimizer::reduceM2P1v4(pencilmarks& forbiddenValuePositions) { // ~1.5 seconds/puzzle
	fprintf(stderr, ".");
//	numSolverCalls = 0;
//	knownNoLockedCandidatesHits = 0;
//	knownNoLockedCandidatesMisses = 0;
//	knownNoHiddenHits = 0;
//	knownNoHiddenMisses = 0;
	getSingleSolution ss;
	struct solution {
		char sol[88];
	};
	solution sol; //pass solution as hint parameter to canonicalizer
	solution originalSol;
	if(1 != ss.solve(forbiddenValuePositions, originalSol.sol)) return; //silently ignore invalid puzzles
	struct redundantInContext {
		int digit;
		int cell;
		bool skip;
		solution sol;
	};
	std::map<std::pair<int,int>,std::list<redundantInContext>> redundantsAlone;
#ifdef COUNT_TRIALS
	int maxTrialsSoFar = 0;
#endif
	//pass 1: collect single-solution puzzles at {-1,+1}
	//apply {+1} and check if single-solution is found
	for(int dAllow = 0; dAllow < 9; dAllow++) {
		for(int cAllow = 0; cAllow < 81; cAllow++) {
			if(!forbiddenValuePositions[dAllow].isBitSet(cAllow)) continue; //skip allowed placements
			forbiddenValuePositions[dAllow].clearBit(cAllow); //allow
			for(int dForbid = 0; dForbid < 9; dForbid++) {
				for(int cForbid = 0; cForbid < 81; cForbid++) {
					//if(originalSol.sol[cForbid] != dForbid) continue; //experimental: enforce solution changing
					if(forbiddenValuePositions[dForbid].isBitSet(cForbid)) continue; //skip already forbidden placements
					if(dForbid == dAllow && cForbid == cAllow) continue; //note that dForbid at cForbid is temporary set
					forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
					//apply {-1} and check if single-solution is found
#ifdef COUNT_TRIALS
							nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, sol.sol);
					if(1 == numSolutions) {
						std::pair<int,int> key(dForbid, cForbid);
						redundantInContext value{dAllow, cAllow, false, sol};
						redundantsAlone[key].push_back(value);
					}
//					else if(dForbid == originalSol.sol[cForbid] - 1 && 0 == numSolutions) { //same solution
//						std::pair<int,int> key(dForbid, cForbid);
//						redundantInContext value{dAllow, cAllow, true, {}};
//						redundantsAlone[key].push_back(value);
//					}
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
						//fprintf(stderr, "\n%d", maxTrialsSoFar);
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),+(%d,%d)}\t%d", nTrials, dAllow, cAllow, dForbid, cForbid, numSolutions);
#endif
					forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
				} //cForbid
			} //dForbid
			forbiddenValuePositions[dAllow].setBit(cAllow); //restore
		} //cAllow
	} //dAllow
	//pass 2: test for {-2,+1}
	for(auto&& m1p1 : redundantsAlone) {
		auto&& m1p1Val = m1p1.second;
		if(m1p1Val.size() < 2) continue;
		auto&& m1p1Key = m1p1.first;
		int dForbid = m1p1Key.first;
		int cForbid = m1p1Key.second;
		forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
		for(auto&& a1 = m1p1Val.begin(); a1 != m1p1Val.end(); a1++) {
			int dAllow1 = a1->digit;
			int cAllow1 = a1->cell;
			forbiddenValuePositions[dAllow1].clearBit(cAllow1); // allow
			auto a2start = a1;
			a2start++;
			for(auto&& a2 = a2start; a2 != m1p1Val.end(); a2++) {
				int dAllow2 = a2->digit;
				int cAllow2 = a2->cell;
				forbiddenValuePositions[dAllow2].clearBit(cAllow2); // allow
#ifdef COUNT_TRIALS
					nTrials = 0;
#endif
					//numSolverCalls++;
					int numSolutions = ss.solve(forbiddenValuePositions, sol.sol);
#ifdef COUNT_TRIALS
					if(maxTrialsSoFar < nTrials) {
						maxTrialsSoFar = nTrials;
					}
					fprintf(stderr, "\n%d\t{-(%d,%d),(%d,%d),+(%d,%d)}\t%d", nTrials, redundantsAlone[dForbid][cForbid].rc[allow1].digit, redundantsAlone[dForbid][cForbid].rc[allow1].cell, redundantsAlone[dForbid][cForbid].rc[allow2].digit, redundantsAlone[dForbid][cForbid].rc[allow2].cell, dForbid, cForbid, numSolutions);
#endif
					if(1 == numSolutions) {
						//lucky
						a1->skip = true;
						a2->skip = true;
						solRowMinLex(forbiddenValuePositions, sol.sol); // export {-2,+1}
						fprintf(stderr, "+");
					}
				forbiddenValuePositions[dAllow2].setBit(cAllow2); // restore
			}
			forbiddenValuePositions[dAllow1].setBit(cAllow1); // restore
		}
		forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
	}
	//pass 3: export minimal {-1,+1}
	for(auto&& m1p1 : redundantsAlone) {
		auto&& m1p1Val = m1p1.second;
		auto&& m1p1Key = m1p1.first;
		int dForbid = m1p1Key.first;
		int cForbid = m1p1Key.second;
		bool sameSolution = dForbid != originalSol.sol[cForbid] - 1;
		forbiddenValuePositions[dForbid].setBit(cForbid); // forbid dForbid in cForbid
		for(auto&& a1 = m1p1Val.begin(); a1 != m1p1Val.end(); a1++) {
			if(a1->skip) continue; //skip non-minimals
			int dAllow1 = a1->digit;
			int cAllow1 = a1->cell;
			forbiddenValuePositions[dAllow1].clearBit(cAllow1); // allow
			if(sameSolution) {
				solRowMinLex(forbiddenValuePositions, a1->sol.sol); // export {-1,+1}
			}
			else {
				//different solution grid can cause non-minimals here
				tryReduceM1(forbiddenValuePositions); // export either {-1,+1} or {-2,+1}
			}
			forbiddenValuePositions[dAllow1].setBit(cAllow1); // restore
		}
		forbiddenValuePositions[dForbid].clearBit(cForbid); // restore
	}
	//fprintf(stderr, "(+-)\t%d\t%d\t%d\t%d\t%d\n", numSolverCalls, knownNoLockedCandidatesHits, knownNoLockedCandidatesMisses, knownNoHiddenHits, knownNoHiddenMisses);
}
void minimizer::tryReduceM1(pencilmarks& forbiddenValuePositions) { //output all unique {-1} if exist, else output original
	//fprintf(stderr, ".");
	getSingleSolution ss;
	isRedundant redundantTester;
	char sol[88]; //pass solution as hint parameter to canonicalizer
	if(1 != ss.solve(forbiddenValuePositions, sol)) return; //silently ignore invalid and miltiple-solution puzzles
	bool hasReduced = false;
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			if(redundantTester.solve(forbiddenValuePositions, d1, c1)) {
				forbiddenValuePositions[d1].clearBit(c1); //allow
				solRowMinLex(forbiddenValuePositions, sol); // export {-1,+1}
				forbiddenValuePositions[d1].setBit(c1); //restore
				hasReduced = true;
			}
		} //c1
	} //d1
	if(hasReduced) {
		fprintf(stderr, "-");
	}
	else {
		solRowMinLex(forbiddenValuePositions, sol); // export original
	}
}
void minimizer::transformM1P1(pencilmarks& forbiddenValuePositions) {
	fprintf(stderr, ".");
	hasSingleSolution ss;
	//apply {-1} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//apply {+1} and check if single-solution is found
			for(int d = 0; d < 9; d++) {
				for(int c = 0; c < 81; c++) {
					if(forbiddenValuePositions[d].isBitSet(c)) continue; //skip already forbidden placements
					if(d == d1 && c == c1) continue;
					forbiddenValuePositions[d].setBit(c); // forbid d in c
					if(1 == ss.solve(forbiddenValuePositions)) {
						//lucky
						tryReduceM1(forbiddenValuePositions);
						//complementaryPencilmarksX::dump2(forbiddenValuePositions);
					}
					forbiddenValuePositions[d].clearBit(c); // restore
				}
			}
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
}
void minimizer::transformM2P2v1(pencilmarks& forbiddenValuePositions) { //full scan
	hasSingleSolution ss;
	int numSolverCalls = 0;
	clock_t start, finish;
	start = clock();
	//apply {-1} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//fprintf(stderr, "(-%d,%d)", d1 + 1, c1);
			//apply {-1} for second time
			for(int d2 = d1; d2 < 9; d2++) {
				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
					forbiddenValuePositions[d2].clearBit(c2); //allow
					//fprintf(stderr, "(--%d,%d)", d2 + 1, c2);
					//apply {+1} and get unavoidable set or single solution
					for(int dd1 = 0; dd1 < 9; dd1++) {
						for(int cc1 = 0; cc1 < 81; cc1++) {
							if(forbiddenValuePositions[dd1].isBitSet(cc1)) continue; //skip already forbidden placements
							if(dd1 == d1 && cc1 == c1) continue; //don't turn d1 back
							if(dd1 == d2 && cc1 == c2) continue; //don't turn d2 back
							forbiddenValuePositions[dd1].setBit(cc1); // forbid dd1 in cc1
							//fprintf(stderr, "(+%d,%d)", dd1 + 1, cc1);
							int numSolP1 = ss.solve(forbiddenValuePositions);
							numSolverCalls++;
							if(1 == numSolP1) {
								//lucky, {-2,+1}
								fprintf(stderr, "+");
								tryReduceM1(forbiddenValuePositions);
								//complementaryPencilmarksX::dump2(forbiddenValuePositions);
							}
							else if(2 == numSolP1) {
								//apply {+1} for second time only on the cells in UA
								for(int dd2 = dd1; dd2 < 9; dd2++) {
									for(int cc2 = dd2 == dd1 ? cc1 + 1 : 0; cc2 < 81; cc2++) {
										if(dd2 == d1 && cc2 == c1) continue; //don't turn d1 back
										if(dd2 == d2 && cc2 == c2) continue; //don't turn d2 back
										forbiddenValuePositions[dd2].setBit(cc2); // forbid dd2 in cc2
										//fprintf(stderr, "(++%d,%d)", dd2 + 1, cc2);
										int numSolP2 = ss.solve(forbiddenValuePositions);
										numSolverCalls++;
										if(1 == numSolP2) {
											//lucky, {-2,+2}
											//complementaryPencilmarksX::dump2(forbiddenValuePositions);
											fprintf(stderr, "=");
											tryReduceM1(forbiddenValuePositions);
										}
										forbiddenValuePositions[dd2].clearBit(cc2); // restore
									} //cc2
								} //dd2
							}
							forbiddenValuePositions[dd1].clearBit(cc1); // restore
						} //cc1
					} //dd1
					forbiddenValuePositions[d2].setBit(c2); //restore
				} //c2
			} //d2
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	finish = clock();
	double tt = ((double)(finish - start)) / CLOCKS_PER_SEC;
	fprintf(stderr, "\t%d,%2.3fs,%dK/s\n", numSolverCalls, tt, (int)(numSolverCalls / tt / 1000));
}
void minimizer::transformM2P2v2(pencilmarks& forbiddenValuePositions) { //partial scan
	hasSingleSolution ss;
	getTwoSolutions ts;
	char sol1[2][81];
	char sol2[2][81];
	int numSolverCalls = 0;
	clock_t start, finish;
	start = clock();
	//apply {-1} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//fprintf(stderr, "(-%d,%d)", d1 + 1, c1);
			//apply {-1} for second time
			for(int d2 = d1; d2 < 9; d2++) {
				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
					forbiddenValuePositions[d2].clearBit(c2); //allow
					//fprintf(stderr, "(--%d,%d)", d2 + 1, c2);
					pencilmarks deadClues;
					deadClues.clear();
					int numSolP0 = ts.solve(forbiddenValuePositions, sol1[0]); //get one deadly pattern
					if(numSolP0 == 2) {
						//apply {+1} and get unavoidable set or single solution
						for(int dd1 = 0; dd1 < 9; dd1++) {
							for(int cc1 = 0; cc1 < 81; cc1++) {
								if(sol1[0][cc1] == sol1[1][cc1]) continue; //skip the cells outside the UA
								if(sol1[0][cc1] != (dd1 + 1) && sol1[1][cc1] != (dd1 + 1)) continue; //skip the values not in the solutions
								if(forbiddenValuePositions[dd1].isBitSet(cc1)) continue; //skip already forbidden placements
								if(dd1 == d1 && cc1 == c1) continue; //don't turn d1 back
								if(dd1 == d2 && cc1 == c2) continue; //don't turn d2 back
								forbiddenValuePositions[dd1].setBit(cc1); // forbid dd1 in cc1
								//fprintf(stderr, "(+%d,%d)", dd1 + 1, cc1);
								int numSolP1 = ts.solve(forbiddenValuePositions, sol2[0]);
								numSolverCalls++;
								if(1 == numSolP1) {
									//lucky, {-2,+1}
									fprintf(stderr, "+");
									tryReduceM1(forbiddenValuePositions);
									//complementaryPencilmarksX::dump2(forbiddenValuePositions);
								}
								else if(2 == numSolP1) {
									//apply {+1} for second time only on the cells in UA
	//								fprintf(stderr, "\n");
	//								for(int cc2 = 0; cc2 < 81; cc2++) {
	//									int dd2 = sol2[0][cc2] - 1;
	//									fprintf(stderr, "%d", dd2 + 1);
	//								}
	//								fprintf(stderr, "\n");
	//								for(int cc2 = 0; cc2 < 81; cc2++) {
	//									int dd2 = sol2[1][cc2] - 1;
	//									fprintf(stderr, "%d", dd2 + 1);
	//								}
	//								fprintf(stderr, "\n");
									for(int cc2 = 0; cc2 < 81; cc2++) {
									//for(int cc2 = cc1; cc2 < 81; cc2++) {
										if(sol2[0][cc2] == sol2[1][cc2]) continue; //skip the cells outside the UA
										//check both solutions
										for(int s = 0; s < 2; s++) {
											int dd2 = sol2[s][cc2] - 1;
											if(dd2 == d1 && cc2 == c1) continue; //don't turn d1 back
											if(dd2 == d2 && cc2 == c2) continue; //don't turn d2 back
											if(deadClues[dd2].isBitSet(cc2)) continue; //it was entirely processed on first {+1} stage
											forbiddenValuePositions[dd2].setBit(cc2); // forbid dd2 in cc2
											//fprintf(stderr, "(++%d,%d)", dd2 + 1, cc2);
											int numSolP2 = ss.solve(forbiddenValuePositions);
											numSolverCalls++;
											if(1 == numSolP2) {
												//lucky, {-2,+2}
												//complementaryPencilmarksX::dump2(forbiddenValuePositions);
												fprintf(stderr, "=");
												tryReduceM1(forbiddenValuePositions);
											}
											forbiddenValuePositions[dd2].clearBit(cc2); // restore
										}
									}
								}
								deadClues[dd1].setBit(cc1); //mark as processed
								forbiddenValuePositions[dd1].clearBit(cc1); // restore
							} //cc1
						} //dd1
					}
					else if(numSolP0 == 1) {
						//{-2,+0} give unique puzzle?
						fprintf(stderr, "!");
						tryReduceM1(forbiddenValuePositions);
					}
					forbiddenValuePositions[d2].setBit(c2); //restore
				} //c2
			} //d2
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	finish = clock();
	double tt = ((double)(finish - start)) / CLOCKS_PER_SEC;
	fprintf(stderr, "\t%d,%2.3fs,%dK/s\n", numSolverCalls, tt, (int)(numSolverCalls / tt / 1000));
}
void minimizer::transformM2P2v3(pencilmarks& forbiddenValuePositions) { //full scan
	hasSingleSolution ss;
	multiSolutionPM as;
	pencilmarks pm1, pm2;
	int numSolverCalls = 0;
	clock_t start, finish;
	start = clock();
	//apply {-1} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//fprintf(stderr, "(-%d,%d)", d1 + 1, c1);
			//apply {-1} for second time
			for(int d2 = d1; d2 < 9; d2++) {
				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
					forbiddenValuePositions[d2].clearBit(c2); //allow
					//fprintf(stderr, "(--%d,%d)", d2 + 1, c2);
					//int numSolP0 = as.solve(forbiddenValuePositions, pm1); //get unsolved pencilmarks
					//if(numSolP0 > 1) {
						//apply {+1} and get pm2 set or single solution
						for(int dd1 = 0; dd1 < 9; dd1++) {
							for(int cc1 = 0; cc1 < 81; cc1++) {
								if(dd1 == d1 && cc1 == c1) continue; //don't turn d1 back
								if(dd1 == d2 && cc1 == c2) continue; //don't turn d2 back
								if(forbiddenValuePositions[dd1].isBitSet(cc1)) continue; //skip already forbidden placements
								//if(!pm1[dd1].isBitSet(cc1)) continue; //skip the eliminated pencilmarks
								forbiddenValuePositions[dd1].setBit(cc1); // forbid dd1 in cc1
								//fprintf(stderr, "(+%d,%d)", dd1 + 1, cc1);
								int numSolP1 = as.solve(forbiddenValuePositions, pm2);
								numSolverCalls++;
								if(1 == numSolP1) {
									//lucky, {-2,+1}
									fprintf(stderr, "+");
									tryReduceM1(forbiddenValuePositions);
									//complementaryPencilmarksX::dump2(forbiddenValuePositions);
								}
								else if(numSolP1 > 1) {
									//apply {+1} for second time only on unsolved pm2
									for(int dd2 = dd1; dd2 < 9; dd2++) {
										for(int cc2 = dd2 == dd1 ? cc1 + 1 : 0; cc2 < 81; cc2++) {
											if(dd2 == d1 && cc2 == c1) continue; //don't turn d1 back
											if(dd2 == d2 && cc2 == c2) continue; //don't turn d2 back
											if(!pm2[dd1].isBitSet(cc1)) continue; //skip the eliminated pencilmarks
											forbiddenValuePositions[dd2].setBit(cc2); // forbid dd2 in cc2
											//fprintf(stderr, "(++%d,%d)", dd2 + 1, cc2);
											int numSolP2 = ss.solve(forbiddenValuePositions);
											numSolverCalls++;
											if(1 == numSolP2) {
												//lucky, {-2,+2}
												//complementaryPencilmarksX::dump2(forbiddenValuePositions);
												fprintf(stderr, "=");
												tryReduceM1(forbiddenValuePositions);
											}
											forbiddenValuePositions[dd2].clearBit(cc2); // restore
										} //cc2
									} //dd2
								}
								forbiddenValuePositions[dd1].clearBit(cc1); // restore
							} //cc1
						} //dd1
					//}
					//else if(numSolP0 == 1) {
					//	//{-2,+0} give unique puzzle?
					//	fprintf(stderr, "!");
					//	tryReduceM1(forbiddenValuePositions);
					//}
					forbiddenValuePositions[d2].setBit(c2); //restore
				} //c2
			} //d2
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	finish = clock();
	double tt = ((double)(finish - start)) / CLOCKS_PER_SEC;
	fprintf(stderr, "\t%d,%2.3fs,%dK/s\n", numSolverCalls, tt, (int)(numSolverCalls / tt / 1000));
}
void minimizer::transformM2P2(pencilmarks& forbiddenValuePositions) { //full scan
	//hasAnySolution sss;
	hasSingleSolution ss;
	int numSolverCalls = 0;
	int p1valid = 0;
	int p1invalid = 0;
	clock_t start, finish;
	start = clock();
	//apply {-1} and get multiple-solution puzzle
	for(int d1 = 0; d1 < 9; d1++) {
		for(int c1 = 0; c1 < 81; c1++) {
			if(!forbiddenValuePositions[d1].isBitSet(c1)) continue; //skip allowed placements
			forbiddenValuePositions[d1].clearBit(c1); //allow
			//fprintf(stderr, "(-%d,%d)", d1 + 1, c1);
			//apply {-1} for second time
			for(int d2 = d1; d2 < 9; d2++) {
				for(int c2 = d1 == d2 ? c1 + 1 : 0; c2 < 81; c2++) {
					if(!forbiddenValuePositions[d2].isBitSet(c2)) continue; //skip allowed placements
					forbiddenValuePositions[d2].clearBit(c2); //allow
					//apply {+1} and get pm2 set or single solution
					for(int dd1 = 0; dd1 < 9; dd1++) {
						for(int cc1 = 0; cc1 < 81; cc1++) {
							if(forbiddenValuePositions[dd1].isBitSet(cc1)) continue; //skip already forbidden placements
							if(dd1 == d1 && cc1 == c1) continue; //don't turn d1 back
							if(dd1 == d2 && cc1 == c2) continue; //don't turn d2 back
							forbiddenValuePositions[dd1].setBit(cc1); // forbid dd1 in cc1
							//fprintf(stderr, "(+%d,%d)", dd1 + 1, cc1);
							int numSolP1 = ss.solve(forbiddenValuePositions);
							numSolverCalls++;
							if(numSolP1 == 2) {
								p1valid++;
								//apply {+1} for second time only on unsolved pm2
//								for(int dd2 = dd1; dd2 < 9; dd2++) {
//									for(int cc2 = dd2 == dd1 ? cc1 + 1 : 0; cc2 < 81; cc2++) {
//										if(dd2 == d1 && cc2 == c1) continue; //don't turn d1 back
//										if(dd2 == d2 && cc2 == c2) continue; //don't turn d2 back
//										if(forbiddenValuePositions[dd2].isBitSet(cc2)) continue; //skip already forbidden placements
//										forbiddenValuePositions[dd2].setBit(cc2); // forbid dd2 in cc2
//										//fprintf(stderr, "(++%d,%d)", dd2 + 1, cc2);
//										int numSolP2 = ss.solve(forbiddenValuePositions);
//										numSolverCalls++;
//										if(1 == numSolP2) {
//											//lucky, {-2,+2}
//											//complementaryPencilmarksX::dump2(forbiddenValuePositions);
//											fprintf(stderr, "=");
//											tryReduceM1(forbiddenValuePositions);
//										}
//										forbiddenValuePositions[dd2].clearBit(cc2); // restore
//									} //cc2
//								} //dd2
							} //any solution
							else {
								p1invalid++;
							}
							forbiddenValuePositions[dd1].clearBit(cc1); // restore
						} //cc1
					} //dd1
					forbiddenValuePositions[d2].setBit(c2); //restore
				} //c2
			} //d2
			forbiddenValuePositions[d1].setBit(c1); //restore
		} //c1
	} //d1
	finish = clock();
	double tt = ((double)(finish - start)) / CLOCKS_PER_SEC;
	fprintf(stderr, "\t%d,%2.3fs,%dK/s(%d/%d)\n", numSolverCalls, tt, (int)(numSolverCalls / tt / 1000), p1valid, p1invalid);
}
void minimizer::solRowMinLex(const pencilmarks& src) { //transform single-solution puzzle to row-min-lex by solution grid
	pencilmarks res;
	if(solRowMinLex(src, res)) {
		complementaryPencilmarksX::dump2(res);
	}
}
bool minimizer::solRowMinLex(const pencilmarks& src, pencilmarks& res) { //transform single-solution puzzle to row-min-lex by solution grid
	getAnySolution solver;
	char sol[88];
	if(0 == solver.solve(src, sol)) return false; //ignore invalid puzzles
	solRowMinLex(src, res, sol);
	return true;
}
void minimizer::solRowMinLex(const pencilmarks& src, pencilmarks& res, const char* sol) { //transform single-solution puzzle to row-min-lex by solution grid
	transformer tr;
	tr.byGrid(sol);
	tr.transform(src, res);
}
void minimizer::solRowMinLex(const pencilmarks& src, const char* sol) { //transform single-solution puzzle to row-min-lex by solution grid
	pencilmarks res;
	transformer tr;
	tr.byGrid(sol);
	tr.transform(src, res);
	complementaryPencilmarksX::dump2(res);
}
void minimizer::guessCounters(const char* p) { //puzzle in 729-columns format, solution, totalNumGuesses, numGuesses[0] ...
	pencilmarks pm;
	singleSolutionGuesses ssg;
	int guessStat[81];
	char sol[88];
	char outPuz[729];
	if(!complementaryPencilmarksX::fromChars2(p, pm)) return;
	//if(!complementaryPencilmarksX::fromChars3(p, pm)) return;
	if(1 == ssg.solve(pm, sol, guessStat)) { //2105 puz/sec no-locked, guess_2
		complementaryPencilmarksX::dump3(pm, outPuz);
		int guessTotal = 0;
		for(int i = 0; i < 81; i++) {
			sol[i] += '0';
			guessTotal += guessStat[i];
		}
		printf("%729.729s\t%81.81s\t%d", outPuz, sol, guessTotal);
		for(int i = 0; i < 81 && guessStat[i]; i++) {
			printf("\t%d", guessStat[i]);
		}
		printf("\n");
	}
}
void minimizer::backdoorSize(const char* p) { //puzzle in 729-columns format, backdoorSize
	pencilmarks pm;
	pencilmarks pmExemplar;
	getSingleSolution ss;
	noGuess ng;
	char sol[88];
	char outPuz[729];
	char outPuz2[729];
	bm128 givens{};
	for(int c = 0; c < 81; c++) {
		int numAllowed = 0;
		for(int d = 0; d < 9; d++) {
			if(!pm[d].isBitSet(c)) {
				if(numAllowed) goto nextCell;
				numAllowed++;
			}
		}
		givens.setBit(c);
		nextCell:;
	}
	//if(!complementaryPencilmarksX::fromChars2(p, pm)) return;
	if(!complementaryPencilmarksX::fromChars3(p, pm)) return;
	if(1 != ss.solve(pm, sol)) return;
	int minBD = 0;
	pmExemplar = pm;
	if(!ng.solve(pm)) {
		minBD = 1;
		for(int c1 = 0; c1 < 81; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			if(ng.solve(pm1)) {
				pmExemplar = pm1;
				goto done;
			}
		}
		minBD = 2;
		for(int c1 = 0; c1 < 81 - 1; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			for(int c2 = c1 + 1; c2 < 81 - 0; c2++) {
				if(givens.isBitSet(c2)) continue;
				pencilmarks pm2(pm1);
				pm2.forceCell(c2, sol[c2] - 1);
				if(ng.solve(pm2)) {
					pmExemplar = pm2;
					goto done;
				}
			}
		}
		minBD = 3;
		for(int c1 = 0; c1 < 81 - 2; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			for(int c2 = c1 + 1; c2 < 81 - 1; c2++) {
				if(givens.isBitSet(c2)) continue;
				pencilmarks pm2(pm1);
				pm2.forceCell(c2, sol[c2] - 1);
				for(int c3 = c2 + 1; c3 < 81 - 0; c3++) {
					if(givens.isBitSet(c3)) continue;
					pencilmarks pm3(pm2);
					pm3.forceCell(c3, sol[c3] - 1);
					if(ng.solve(pm3)) {
						pmExemplar = pm3;
						goto done;
					}
				}
			}
		}
		minBD = 4;
		for(int c1 = 0; c1 < 81 - 3; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			for(int c2 = c1 + 1; c2 < 81 - 2; c2++) {
				if(givens.isBitSet(c2)) continue;
				pencilmarks pm2(pm1);
				pm2.forceCell(c2, sol[c2] - 1);
				for(int c3 = c2 + 1; c3 < 81 - 1; c3++) {
					if(givens.isBitSet(c3)) continue;
					pencilmarks pm3(pm2);
					pm3.forceCell(c3, sol[c3] - 1);
					for(int c4 = c3 + 1; c4 < 81 - 0; c4++) {
						if(givens.isBitSet(c4)) continue;
						pencilmarks pm4(pm3);
						pm4.forceCell(c4, sol[c4] - 1);
						if(ng.solve(pm4)) {
							pmExemplar = pm4;
							goto done;
						}
					}
				}
			}
		}
		minBD = 5;
		for(int c1 = 0; c1 < 81 - 4; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			for(int c2 = c1 + 1; c2 < 81 - 3; c2++) {
				if(givens.isBitSet(c2)) continue;
				pencilmarks pm2(pm1);
				pm2.forceCell(c2, sol[c2] - 1);
				for(int c3 = c2 + 1; c3 < 81 - 2; c3++) {
					if(givens.isBitSet(c3)) continue;
					pencilmarks pm3(pm2);
					pm3.forceCell(c3, sol[c3] - 1);
					for(int c4 = c3 + 1; c4 < 81 - 1; c4++) {
						if(givens.isBitSet(c4)) continue;
						pencilmarks pm4(pm3);
						pm4.forceCell(c4, sol[c4] - 1);
						for(int c5 = c4 + 1; c5 < 81 - 0; c5++) {
							if(givens.isBitSet(c5)) continue;
							pencilmarks pm5(pm4);
							pm5.forceCell(c5, sol[c5] - 1);
							if(ng.solve(pm5)) {
								pmExemplar = pm5;
								goto done;
							}
						}
					}
				}
			}
		}
		minBD = 6;
		for(int c1 = 0; c1 < 81 - 5; c1++) {
			if(givens.isBitSet(c1)) continue;
			pencilmarks pm1(pm);
			pm1.forceCell(c1, sol[c1] - 1);
			for(int c2 = c1 + 1; c2 < 81 - 4; c2++) {
				if(givens.isBitSet(c2)) continue;
				pencilmarks pm2(pm1);
				pm2.forceCell(c2, sol[c2] - 1);
				for(int c3 = c2 + 1; c3 < 81 - 3; c3++) {
					if(givens.isBitSet(c3)) continue;
					pencilmarks pm3(pm2);
					pm3.forceCell(c3, sol[c3] - 1);
					for(int c4 = c3 + 1; c4 < 81 - 2; c4++) {
						if(givens.isBitSet(c4)) continue;
						pencilmarks pm4(pm3);
						pm4.forceCell(c4, sol[c4] - 1);
						for(int c5 = c4 + 1; c5 < 81 - 1; c5++) {
							if(givens.isBitSet(c4)) continue;
							pencilmarks pm5(pm4);
							pm5.forceCell(c5, sol[c5] - 1);
							for(int c6 = c5 + 1; c6 < 81 - 0; c6++) {
								if(givens.isBitSet(c6)) continue;
								pencilmarks pm6(pm5);
								pm5.forceCell(c6, sol[c6] - 1);
								if(ng.solve(pm6)) {
									pmExemplar = pm6;
									goto done;
								}
							}
						}
					}
				}
			}
		}
		minBD = 99;
	}
	done:
	complementaryPencilmarksX::dump3(pm, outPuz);
	complementaryPencilmarksX::dump3(pmExemplar, outPuz2);
//	for(int i = 0; i < 81; i++) {
//		sol[i] += '0';
//	}
//	printf("%729.729s\t%81.81s\t%d\n", outPuz, sol, minBD);
	printf("%729.729s\t%d\t%729.729s\n", outPuz, minBD, outPuz2);
	fflush(NULL);
}
void minimizer::backdoorSizePm(const char* p) { //puzzle in 729-columns format, backdoorSizePm
	struct pmIter {
		int d = 0;
		int c = 0;
		int index = 0;
		pmIter(int index_) : d(index_ / 81), c(index_ % 81), index(index_) {}
		pmIter& operator++() {
			index++;
			if(c == 80) {
				d++;
				c = 0;
			}
			else {
				c++;
			}
			return *this;
		}
		bool operator< (int i) const {
			return index < i;
		}
	};
//	struct s {
//		int trySolving(int depth, pencilmarks& pm, const bm128& givens, int start, pencilmarks& exemplar, const char* solution) {
//			noGuess ng;
//			if(ng.reduce(pm)) {
//				pm.allowSolution(solution);
//				exemplar = pm;
//				return depth;
//			}
//			pencilmarks reducedPm[729];
//			for(pmIter f(start); f < 729; ++f) {
//				if(givens.isBitSet(f.c)) continue; //initial given
//				if(pm[f.d].isBitSet(f.c)) continue; //already forbidden
//				pencilmarks pm1(pm);
//				pm1[f1.d].setBit(f1.c); //forbid
//				if(ng.solve(pm1)) {
//					pm1.allowSolution(solution);
//					exemplar = pm1;
//					return depth + 1;
//				}
//				pm1.allowSolution(solution);
//				reducedPm[f.index] = pm1;
//			}
//			return trySolving(depth + 1, pm1, givens, f.index + 1, exemplar, solution);
//		}
//	};
	pencilmarks pm;
	pencilmarks pmExemplar;
	getSingleSolution ss;
	noGuess ng;
	char sol[88];
	char outPuz[729];
	char outPuz2[729];
//	bm128 givens{};
//	for(int c = 0; c < 81; c++) {
//		int numAllowed = 0;
//		for(int d = 0; d < 9; d++) {
//			if(!pm[d].isBitSet(c)) {
//				if(numAllowed) goto nextCell;
//				numAllowed++;
//			}
//		}
//		givens.setBit(c);
//		nextCell:;
//	}
	//if(!complementaryPencilmarksX::fromChars2(p, pm)) return;
	if(!complementaryPencilmarksX::fromChars3(p, pm)) return;
	if(1 != ss.solve(pm, sol)) return;
	complementaryPencilmarksX::dump3(pm, outPuz);
	int minBD = 0;
	pmExemplar = pm;

	if(!ng.solve(pm)) {
		minBD = 1;
		for(pmIter f1(0); f1 < 729; ++f1) {
			if(sol[f1.c] - 1 == f1.d) continue;
			if(pm[f1.d].isBitSet(f1.c)) continue;
			pencilmarks pm1(pm);
			pm1[f1.d].setBit(f1.c);
			if(ng.solve(pm1)) {
				pmExemplar = pm;
				pmExemplar[f1.d].setBit(f1.c);
				goto done;
			}
		}
		minBD = 2;
		for(pmIter f1(0); f1 < 729 - 1; ++f1) {
			if(sol[f1.c] - 1 == f1.d) continue;
			if(pm[f1.d].isBitSet(f1.c)) continue;
			pencilmarks pm1(pm);
			pm1[f1.d].setBit(f1.c);
			ng.reduce(pm1);
			pm1.allowSolution(sol);
			for(pmIter f2(f1.index + 1); f2 < 729 - 0; ++f2) {
				if(sol[f2.c] - 1 == f2.d) continue;
				if(pm1[f2.d].isBitSet(f2.c)) continue;
				pencilmarks pm2(pm1);
				pm2[f2.d].setBit(f2.c);
				if(ng.solve(pm2)) {
					pmExemplar = pm;
					pmExemplar[f1.d].setBit(f1.c);
					pmExemplar[f2.d].setBit(f2.c);
					goto done;
				}
			}
		}
		minBD = 3;
		for(pmIter f1(0); f1 < 729 - 2; ++f1) {
			if(sol[f1.c] - 1 == f1.d) continue;
			if(pm[f1.d].isBitSet(f1.c)) continue;
			pencilmarks pm1(pm);
			pm1[f1.d].setBit(f1.c);
			ng.reduce(pm1);
			pm1.allowSolution(sol);
			for(pmIter f2(f1.index + 1); f2 < 729 - 1; ++f2) {
				if(sol[f2.c] - 1 == f2.d) continue;
				if(pm1[f2.d].isBitSet(f2.c)) continue;
				pencilmarks pm2(pm1);
				pm2[f2.d].setBit(f2.c);
				ng.reduce(pm2);
				pm2.allowSolution(sol);
				for(pmIter f3(f2.index + 1); f3 < 729 - 0; ++f3) {
					if(sol[f3.c] - 1 == f3.d) continue;
					if(pm2[f3.d].isBitSet(f3.c)) continue;
					pencilmarks pm3(pm2);
					pm3[f3.d].setBit(f3.c);
					if(ng.solve(pm3)) {
						pmExemplar = pm;
						pmExemplar[f1.d].setBit(f1.c);
						pmExemplar[f2.d].setBit(f2.c);
						pmExemplar[f3.d].setBit(f3.c);
						goto done;
					}
				}
			}
		}
		minBD = 4;
		for(pmIter f1(0); f1 < 729 - 3; ++f1) {
			if(sol[f1.c] - 1 == f1.d) continue;
			if(pm[f1.d].isBitSet(f1.c)) continue;
			pencilmarks pm1(pm);
			pm1[f1.d].setBit(f1.c);
			ng.reduce(pm1);
			pm1.allowSolution(sol);
			for(pmIter f2(f1.index + 1); f2 < 729 - 2; ++f2) {
				if(sol[f2.c] - 1 == f2.d) continue;
				if(pm1[f2.d].isBitSet(f2.c)) continue;
				pencilmarks pm2(pm1);
				pm2[f2.d].setBit(f2.c);
				ng.reduce(pm2);
				pm2.allowSolution(sol);
				for(pmIter f3(f2.index + 1); f3 < 729 - 1; ++f3) {
					if(sol[f3.c] - 1 == f3.d) continue;
					if(pm2[f3.d].isBitSet(f3.c)) continue;
					pencilmarks pm3(pm2);
					pm3[f3.d].setBit(f3.c);
					ng.reduce(pm3);
					pm3.allowSolution(sol);
					for(pmIter f4(f3.index + 1); f4 < 729 - 0; ++f4) {
						if(sol[f4.c] - 1 == f4.d) continue;
						if(pm3[f4.d].isBitSet(f4.c)) continue;
						pencilmarks pm4(pm3);
						pm4[f4.d].setBit(f4.c);
						if(ng.solve(pm4)) {
							pmExemplar = pm;
							pmExemplar[f1.d].setBit(f1.c);
							pmExemplar[f2.d].setBit(f2.c);
							pmExemplar[f3.d].setBit(f3.c);
							pmExemplar[f4.d].setBit(f4.c);
							goto done;
						}
					}
				}
			}
		}
		minBD = 5;
		for(pmIter f1(0); f1 < 729 - 4; ++f1) {
			if(sol[f1.c] - 1 == f1.d) continue;
			if(pm[f1.d].isBitSet(f1.c)) continue;
			pencilmarks pm1(pm);
			pm1[f1.d].setBit(f1.c);
			ng.reduce(pm1);
			pm1.allowSolution(sol);
			for(pmIter f2(f1.index + 1); f2 < 729 - 3; ++f2) {
				if(sol[f2.c] - 1 == f2.d) continue;
				if(pm1[f2.d].isBitSet(f2.c)) continue;
				pencilmarks pm2(pm1);
				pm2[f2.d].setBit(f2.c);
				ng.reduce(pm2);
				pm2.allowSolution(sol);
				for(pmIter f3(f2.index + 1); f3 < 729 - 2; ++f3) {
					if(sol[f3.c] - 1 == f3.d) continue;
					if(pm2[f3.d].isBitSet(f3.c)) continue;
					pencilmarks pm3(pm2);
					pm3[f3.d].setBit(f3.c);
					ng.reduce(pm3);
					pm3.allowSolution(sol);
					for(pmIter f4(f3.index + 1); f4 < 729 - 1; ++f4) {
						if(sol[f4.c] - 1 == f4.d) continue;
						if(pm3[f4.d].isBitSet(f4.c)) continue;
						pencilmarks pm4(pm3);
						pm4[f4.d].setBit(f4.c);
						ng.reduce(pm4);
						pm4.allowSolution(sol);
						for(pmIter f5(f4.index + 1); f5 < 729 - 0; ++f5) {
							if(sol[f5.c] - 1 == f5.d) continue;
							if(pm4[f5.d].isBitSet(f5.c)) continue;
							pencilmarks pm5(pm4);
							pm5[f5.d].setBit(f5.c);
							if(ng.solve(pm5)) {
								pmExemplar = pm;
								pmExemplar[f1.d].setBit(f1.c);
								pmExemplar[f2.d].setBit(f2.c);
								pmExemplar[f3.d].setBit(f3.c);
								pmExemplar[f4.d].setBit(f4.c);
								pmExemplar[f5.d].setBit(f5.c);
								goto done;
							}
						}
					}
				}
			}
		}

		minBD = 999;
	}
	done:
	complementaryPencilmarksX::dump3(pmExemplar, outPuz2);
	printf("%729.729s\t%d\t%729.729s\n", outPuz, minBD, outPuz2);
	//debug
	if(minBD != 999 && ng.solve(pmExemplar) == 0) printf("Bug: above is unsolvable\n");
	fflush(NULL);
}
void minimizer::solve(const char* p) { //puzzle in 729-columns format, solution
	pencilmarks pm;
	getSingleSolution ss;
	char sol[88];
	char outPuz[729];
	//if(!complementaryPencilmarksX::fromChars2(p, pm)) return;
	if(!complementaryPencilmarksX::fromChars3(p, pm)) return;
	if(1 != ss.solve(pm, sol)) return;
	complementaryPencilmarksX::dump3(pm, outPuz);
	for(int i = 0; i < 81; i++) {
		sol[i] += '0';
	}
	printf("%729.729s\t%81.81s\n", outPuz, sol);
}
#endif
