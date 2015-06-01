//
// random number generator copied straight from pbrt
//
#pragma once

// Random Number Method Definitions
#define MATH_RNG_M 397
#define MATH_RNG_MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define MATH_RNG_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define MATH_RNG_LOWER_MASK 0x7fffffffUL /* least significant r bits */

namespace houio
{


namespace math
{

	class RNG
	{
	public:
		RNG(unsigned int seed = 5489UL)
		{
			mti = N+1; /* mti==N+1 means mt[N] is not initialized */
			Seed(seed);
		}

		void Seed(unsigned int seed) const
		{
			mt[0]= seed & 0xffffffffUL;
			for (mti=1; mti<N; mti++)
			{
				mt[mti] =
				(1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
				/* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
				/* In the previous versions, MSBs of the seed affect   */
				/* only MSBs of the array mt[].                        */
				/* 2002/01/09 modified by Makoto Matsumoto             */
				mt[mti] &= 0xffffffffUL;
				/* for >32 bit machines */
			}
		}

		float randomFloat() const
		{
			float v = (randomUInt() & 0xffffff) / float(1 << 24);
			return v;
		}
		unsigned long randomUInt() const
		{
			unsigned long y;
			static unsigned long mag01[2]={0x0UL, MATH_RNG_MATRIX_A};
			/* mag01[x] = x * MATRIX_A  for x=0,1 */

			if (mti >= N) { /* generate N words at one time */
				int kk;

				if (mti == N+1)   /* if Seed() has not been called, */
					Seed(5489UL); /* default initial seed */

				for (kk=0;kk<N-MATH_RNG_M;kk++) {
					y = (mt[kk]&MATH_RNG_UPPER_MASK)|(mt[kk+1]&MATH_RNG_LOWER_MASK);
					mt[kk] = mt[kk+MATH_RNG_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
				}
				for (;kk<N-1;kk++) {
					y = (mt[kk]&MATH_RNG_UPPER_MASK)|(mt[kk+1]&MATH_RNG_LOWER_MASK);
					mt[kk] = mt[kk+(MATH_RNG_M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
				}
				y = (mt[N-1]&MATH_RNG_UPPER_MASK)|(mt[0]&MATH_RNG_LOWER_MASK);
				mt[N-1] = mt[MATH_RNG_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

				mti = 0;
			}

			y = mt[mti++];

			/* Tempering */
			y ^= (y >> 11);
			y ^= (y << 7) & 0x9d2c5680UL;
			y ^= (y << 15) & 0xefc60000UL;
			y ^= (y >> 18);

			return y;
		}

		/// use the parethesis operator to get a new random value
		inline float operator()()
		{
			// same as randomFloat
			float v = (randomUInt() & 0xffffff) / float(1 << 24);
			return v;
		}



		private:
		static const int N = 624;
		mutable unsigned long mt[N]; /* the array for the state vector  */
		mutable int mti;
	};










	///  global random number generator for fast and easy random-number-access
	static RNG g_randomNumber;
















	/*

	// old implementation based on  Random Number Generator class from Realistic Ray Tracing 2nd Ed. (Uses a linear congruence algorithm)

	typedef unsigned long long ull_t;

	///
	/// \brief the Random Number Generator may be used to generate a random number within [0,1]
	///
	class RNG
	{
    public:
        RNG(unsigned long long _seed = 7564231ULL)
        {
            seed = _seed;
            mult = 62089911ULL;
            llong_max = 4294967295ULL;
            float_max = 4294967295.0f;
        }

        float operator()();
        void setSeed(unsigned long long _seed = 7564231ULL);
        
        unsigned long long seed;
        unsigned long long mult;
        unsigned long long llong_max;
        float float_max;
	};

	/// use the parethesis operator to get a new random value
	inline float RNG::operator()()
	{
		seed = mult * seed;
		return float(seed % llong_max) / float_max;
	}

	inline void RNG::setSeed(unsigned long long _seed)
	{
		seed = _seed;
	}



	*/



}

} // namespace houio
