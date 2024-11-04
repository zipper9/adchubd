#include "stdinc.h"
#include "Random.h"
#include "Locks.h"

#ifdef HAVE_OPENSSL
#include <openssl/rand.h>
#else
#include "TimeUtil.h"
#endif

/* Below is a high-speed random number generator with much
   better granularity than the CRT one in msvc...(no, I didn't
   write it...see copyright) */
/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.
   Any feedback is very welcome. For any question, comments,
   see http://www.math.keio.ac.jp/matumoto/emt.html or email
   matumoto@math.keio.ac.jp */

static Util::MT19937 randState;
static FastCriticalSection csRandState;

// No locking!
void Util::initRand()
{
	randState.randomize();
}

uint32_t Util::rand()
{
	LOCK(csRandState);
	return randState.getValue();
}

void Util::MT19937::randomize()
{
#ifdef HAVE_OPENSSL
	RAND_pseudo_bytes((unsigned char*) mt, sizeof(mt));
	mti = N + 1;
#else
	uint32_t seed = 0;
	while (seed == 0)
		seed = (uint32_t) time(nullptr) ^ (uint32_t) Util::getHighResTimestamp();
	setSeed(seed);
#endif
}

void Util::MT19937::setSeed(uint32_t seed)
{
	/* setting initial seeds to mt[N] using         */
	/* the generator Line 25 of Table 1 in          */
	/* [KNUTH 1981, The Art of Computer Programming */
	/*    Vol. 2 (2nd Ed.), pp102]                  */
	mt[0] = seed;
	for (mti = 1; mti < N; mti++)
		mt[mti] = 69069 * mt[mti-1];
	mti = N + 1;
}

/* Period parameters */
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

uint32_t Util::MT19937::getValue()
{
	uint32_t y;
	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= N)   /* generate N words at one time */
	{
		static const uint32_t mag01[2] = {0x0, MATRIX_A};
		int kk;

		if (mti == N + 1)
			initRand();

		for (kk = 0; kk < N - M; kk++)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (; kk < N - 1; kk++)
		{
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);

	return y;
}
