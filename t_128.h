#ifndef T_128_H_INCLUDED

#define T_128_H_INCLUDED

#include <smmintrin.h>
#include <limits.h>

#define _popcnt64(a) _mm_popcnt_u64(a)

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
extern const t_128 maskLSB[129];
extern const t_128 maskffff;
static const unsigned char toPos[] = {
	0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
};

struct bm128 {
private:
	inline static bool equals(const __m128i l, const __m128i r) {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(l, r));};
public:
	t_128 bitmap128;
	bm128() {};
	bm128(const bm128 &v) {bitmap128.m128i_m128i = v.bitmap128.m128i_m128i;};
	bm128(const __m128i &v) {bitmap128.m128i_m128i = v;};
	bm128(const t_128 &v) {bitmap128.m128i_m128i = v.m128i_m128i;};
	inline bool operator== (const bm128& r) const {return 0xFFFF == _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i));};
	inline void operator&= (const bm128& r) {bitmap128.m128i_m128i = _mm_and_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	inline void operator|= (const bm128& r) {bitmap128.m128i_m128i = _mm_or_si128(bitmap128.m128i_m128i, r.bitmap128.m128i_m128i);};
	//inline bool isDisjoint(const bm128& r) const {return equals(andnot(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i), bitmap128.m128i_m128i);};
	//inline bool isDisjoint(const bm128& r) const {return 0 == ((r.bitmap128.m128i_u64[0] & bitmap128.m128i_u64[0]) | (r.bitmap128.m128i_u64[1] & bitmap128.m128i_u64[1]));};
	inline bool isDisjoint(const bm128& r) const {return _mm_testz_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline int mask8() const {return _mm_movemask_epi8(bitmap128.m128i_m128i);}
	inline int toInt32() const {return _mm_cvtsi128_si32(bitmap128.m128i_m128i);}
	inline int toInt64() const {return _mm_cvtsi128_si64(bitmap128.m128i_m128i);}
	inline unsigned long long toInt64_1() const {return _mm_extract_epi64(bitmap128.m128i_m128i, 1);}
	inline bool isBitSet(const int theBit) const {return !_mm_testz_si128(this->bitmap128.m128i_m128i, bitSet[theBit].m128i_m128i);};
	inline void setBit(const int theBit) {*this |= bitSet[theBit].m128i_m128i;};
	inline void clearBit(const int theBit) {bitmap128.m128i_m128i = _mm_andnot_si128(bitSet[theBit].m128i_m128i, bitmap128.m128i_m128i);};
	inline void clearBits(const bm128& r) {bitmap128.m128i_m128i = _mm_andnot_si128(r.bitmap128.m128i_m128i, bitmap128.m128i_m128i);};
	inline void clear() {bitmap128.m128i_m128i = _mm_setzero_si128();};
	inline bool isSubsetOf(const bm128 &s) const {return _mm_testc_si128(s.bitmap128.m128i_m128i, bitmap128.m128i_m128i);}
	inline bool operator< (const bm128 &rhs) const {
		if(bitmap128.m128i_u64[1] < rhs.bitmap128.m128i_u64[1]) return true;
		if(bitmap128.m128i_u64[1] > rhs.bitmap128.m128i_u64[1]) return false;
		return bitmap128.m128i_u64[0] < rhs.bitmap128.m128i_u64[0];
	}
	inline void operator= (const bm128 &rhs) {bitmap128.m128i_m128i = rhs.bitmap128.m128i_m128i;};
	inline bool isZero() const {return _mm_testc_si128(_mm_setzero_si128(), bitmap128.m128i_m128i);};
	//void toMask81(char* r) const {for(int i = 0; i < 81; i++) r[i] = isBitSet(i) ? '1' : '.';}
	//void toMask128(char* r) const {for(int i = 0; i < 128; i++) r[i] = isBitSet(i) ? '1' : '.';}
	inline int popcount_128() const {
		long long i, j;
		__m128i xmm = bitmap128.m128i_m128i;
		i = _mm_cvtsi128_si64(xmm);
		xmm = _mm_srli_si128(xmm, 8);
		j = _mm_cvtsi128_si64(xmm);
		return (_popcnt64(i) + _popcnt64(j));
	}
    inline unsigned int nonzeroOctets() const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, _mm_setzero_si128()));}
    inline unsigned int diffOctets(const bm128 &rhs) const {return 0x0000ffff ^ _mm_movemask_epi8(_mm_cmpeq_epi8(bitmap128.m128i_m128i, rhs.bitmap128.m128i_m128i));}
	int getFirstBit1Index96() const {
		uint32_t i;
		bm128 xmm = bitmap128.m128i_m128i;
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		if(i) {
			return __builtin_ctz(i);
		}
		xmm.bitmap128.m128i_m128i = _mm_srli_si128(xmm.bitmap128.m128i_m128i, 4);
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		if(i) {
			return 32 + __builtin_ctz(i);
		}
		xmm.bitmap128.m128i_m128i = _mm_srli_si128(xmm.bitmap128.m128i_m128i, 4);
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		if(i) {
			return 64 + __builtin_ctz(i);
		}
		return -1;
	}
	int getPositions96(unsigned char *positions) const {
		uint32_t i;
		int n = 0;
		bm128 xmm = bitmap128.m128i_m128i;
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		while(i) {
			positions[n++] = __builtin_ctz(i);
			i ^= (i & -i);
		}
		xmm.bitmap128.m128i_m128i = _mm_srli_si128(xmm.bitmap128.m128i_m128i, 4);
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		while(i) {
			positions[n++] = 32 + __builtin_ctz(i);
			i ^= (i & -i);
		}
		xmm.bitmap128.m128i_m128i = _mm_srli_si128(xmm.bitmap128.m128i_m128i, 4);
		i = _mm_cvtsi128_si32(xmm.bitmap128.m128i_m128i);
		while(i) {
			positions[n++] = 64 + __builtin_ctz(i);
			i ^= (i & -i);
		}
		return n;
	}
	int findSingleBitIndex96() const {
		//if (x & (x-1)) == 0 then x has 0 or 1 bit set
		t_128 one = {1,1};
		if(0 == _mm_testz_si128(bitmap128.m128i_m128i, _mm_sub_epi64(bitmap128.m128i_m128i, one.m128i_m128i)))
			return -1;

		unsigned long long i;
		uint32_t j;
		i = _mm_cvtsi128_si64(bitmap128.m128i_m128i);
		j = _mm_cvtsi128_si32(_mm_srli_si128(bitmap128.m128i_m128i, 8));
		if(i) {
			if(j) return -1;
			return __builtin_ctzll(i);
		}
		if(j) {
			return 64 + __builtin_ctz(j);
		}
		return -2; //no any candidate for the given digit in the given unsolved house
	}
};

#endif // T_128_H_INCLUDED
