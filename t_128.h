/*
 * t_128.h
 *
 *  Created on: May 13, 2014
 *      Author: Mladen Dobrichev
 */

//wrapper to 128-bit SIMD x86 instructions

#ifndef T_128_H_INCLUDED

#define T_128_H_INCLUDED

#include <immintrin.h>
#include <limits.h>

//#define _popcnt64(a) _mm_popcnt_u64(a)
#define _popcnt64(a) __builtin_popcountll(a)

#ifndef _MSC_VER
	//assume every compiler but MS is C99 compliant and has inttypes
	#include <inttypes.h>
#else
   //typedef signed __int8     int8_t;
   //typedef signed __int16    int16_t;
   //typedef signed __int32    int32_t;
   typedef unsigned __int8   uint8_t;
   typedef unsigned __int16  uint16_t;
   typedef unsigned __int32  uint32_t;
   //typedef signed __int64       int64_t;
   typedef unsigned __int64     uint64_t;
#endif

typedef union t_128 {
    ////unsigned __int64    m128i_u64[2];
    uint64_t    m128i_u64[2];
    uint8_t     m128i_u8[16];
    uint16_t    m128i_u16[8];
    uint32_t    m128i_u32[4];
	__m128i				m128i_m128i;
    //__int64             m128i_i64[2];
	//__m128d				m128i_m128d;
} t_128;

extern const t_128 bitSet[128];
//extern const t_128 maskLSB[129];
//extern const t_128 mask81;
//extern const t_128 mask108;
//extern const t_128 mask109;

struct bm128 {
	t_128 bitmap128;
	bm128() {};
	bm128(const bm128 &v) {bitmap128.m128i_m128i = v.bitmap128.m128i_m128i;};
	bm128(const __m128i &v) {bitmap128.m128i_m128i = v;};
	bm128(const t_128 &v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	inline bool operator== (const bm128& r) const {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline void operator&= (const bm128& r) {bitmap128.m128i_m128i = _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const bm128& r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator^= (const bm128& r) {bitmap128.m128i_m128i = _mm_xor_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline bool isDisjoint(const bm128& r) const {return _mm_testz_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	//inline int mask8() const {return _mm_movemask_epi8(bitmap128.m128i_m128i);}
	//inline int toInt32() const {return _mm_cvtsi128_si32(bitmap128.m128i_m128i);}
	inline uint64_t toInt64() const {return _mm_cvtsi128_si64(bitmap128.m128i_m128i);}
	inline uint64_t toInt64_1() const {return _mm_extract_epi64(bitmap128.m128i_m128i, 1);}
	inline int toInt32_2() const {return _mm_extract_epi32(bitmap128.m128i_m128i, 2);}
	//inline int toInt32_2() const {return _mm_cvtsi128_si32(_mm_srli_si128(bitmap128.m128i_m128i, 8));}
	inline bool isBitSet(const int theBit) const {return !_mm_testz_si128(this->bitmap128.m128i_m128i, bitSet[theBit].m128i_m128i);};
	inline void setBit(const int theBit) {*this |= bitSet[theBit].m128i_m128i;};
	inline void clearBit(const int theBit) {bitmap128.m128i_m128i = _mm_andnot_si128(bitSet[theBit].m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline void clear() {bitmap128.m128i_m128i = _mm_setzero_si128();};
	inline bool isSubsetOf(const bm128 &s) const {return _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	inline void operator= (const bm128 &rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
	inline bool isZero() const {return _mm_testc_si128(_mm_setzero_si128(), bitmap128.m128i_m128i);};
	//void toMask81(char* r) const {for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? '1' : '.';}
	//void toMask128(char* r) const {for(int i = 0; i < 128; i++) r[i] = isBitSet(i) ? '1' : '.';}
	inline int popcount_128() const {
		uint64_t i, j;
		__m128i xmm = bitmap128.m128i_m128i;
		i = _mm_cvtsi128_si64(xmm);
		xmm = _mm_srli_si128(xmm, 8);
		j = _mm_cvtsi128_si64(xmm);
		return (_popcnt64(i) + _popcnt64(j));
	}
    inline unsigned int nonzeroOctets() const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, _mm_setzero_si128()));}
    inline unsigned int diffOctets(const bm128 &rhs) const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, rhs.bitmap128.m128i_m128i));}
	inline int getFirstBit1Index96() const {
		//lower 64 bits
		{
			uint64_t i = toInt64();
			if(i) {
				return FindLSBIndex64(i);
			}
		}
		//upper 32 bits
		{
			bm128 xmm;
			xmm.bitmap128.m128i_m128i = _mm_srli_si128(bitmap128.m128i_m128i, 8);
			uint32_t i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
			if(i) {
				return 64 + __builtin_ctz(i);
			}
		}
		return -1;
	}
//	inline int hasMax2Bits() const {
//		//exploit the fact that when (x & (x-1)) == 0 then x has 0 or 1 bits set
//		static const t_128 minus1 = {0xffffffffffffffff,0xffffffffffffffff};
//		return _mm_testz_si128(bitmap128.m128i_m128i, _mm_add_epi64(bitmap128.m128i_m128i, minus1.m128i_m128i));
//	}
    inline static uint64_t FindLSBIndex64(const uint64_t Mask) {
    	uint64_t Ret;
        __asm__
        (
            "bsfq %[Mask], %[Ret]"
            :[Ret] "=r" (Ret)
            :[Mask] "mr" (Mask)
        );
        return Ret;
        //return __builtin_ctzll(Mask);
    }
    inline static unsigned int FindLSBIndex32(const uint32_t Mask) {
        return __builtin_ctz(Mask);
    }
};

#endif // T_128_H_INCLUDED
