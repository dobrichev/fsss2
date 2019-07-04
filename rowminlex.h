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

#ifndef ROWMINLEX_H_INCLUDED

#define ROWMINLEX_H_INCLUDED

#include <memory.h>
#include "t_128.h"

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

struct transformer
{
	struct transformationConstants
	{
		unsigned char	swap[18][9][9];
		unsigned char	perm[6][3];
		unsigned char	part[9][5];
		unsigned char	boxOffset[9];
	};
	static const transformer::transformationConstants tc;

	unsigned int	box;
	unsigned int	map[10];
	unsigned int	row[9];
	unsigned int	col[9];
	unsigned int	aut;
	transformer *next; //0 terminated chain of automorphic transformations for aut > 1

	void byGrid(const char* sol);
	//void byPuzzle(const char* sol);
	void transform(const char *in, char *out) const;
	void transform(const bm128 *in, bm128 *out) const;
	void reverseTransform(const char *in, char *out) const;
	//void toString(char *buf) const;
	transformer() : box(0), aut(0) {
		next = 0;
	}
	const transformer & operator=(const transformer &t) {
		memcpy((transformer*)this, &t, sizeof(transformer));
		if(t.next) {
			transformer *t = new transformer();
			*t = *next;
			next = t;
		}
		return *this;
	}
	~transformer() {
		if(next) {
			delete next;
		}
	}
	NOINLINE void addAutomorph(transformer *test) {
		transformer *t = new transformer();
		*t = *test;
		t->map[0] = 0;
		t->next = next; //???
		next = t;
	}
	NOINLINE void clearAutomorphs() {
		if(next) {
			next->clearAutomorphs();
			next = 0;
		}
	}
	bool isTransforming() const {
		if(aut != 1) return true;
		if(box) return true;
		for(unsigned int i = 0; i < 9; i++) {
			if(row[i] != i || col[i] != i || map[i] != i) return true;
		}
		if(map[9] != 9) return true;
		return false;
	}
};
#endif // ROWMINLEX_H_INCLUDED
