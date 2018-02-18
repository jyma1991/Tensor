/*
Copyright (c) 2017, LibMars Developers.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MARS_TYPE_RANDOMGENERATOR_H
#define MARS_TYPE_RANDOMGENERATOR_H 1

namespace mars{

class RandomGenerator
{
private:
    gsl_rng* m_rng;

    void init(gsl_rng*& r, int seed)    // w/o reference &, this is a bug.
    {
        r = gsl_rng_alloc (gsl_rng_default);

        gsl_rng_set( r, seed );
        gsl_rng_env_setup();

        if(m_rng == nullptr)
        {
            printf("Random not initialized\n");
            printf("mars::random::global_gsl_rng=%p\n", m_rng);
            mars_log_code();
            exit(1);
        }
    }

    void end(gsl_rng* r) { gsl_rng_free(r); }

    public:
    RandomGenerator(){init(m_rng, 123456);}
    RandomGenerator(long int seed) { init(m_rng, seed); }
    ~RandomGenerator() { end(m_rng); }
    inline double uniform(double a, double b) { return( gsl_ran_flat(m_rng, a, b) ); }
    inline double uniform() { return( gsl_ran_flat(m_rng, 0, 1) ); }
    inline double normal(double mu, double sigma) { return(mu + sigma * gsl_ran_ugaussian(m_rng)); }
    inline double normal() { return(gsl_ran_ugaussian(m_rng)); }
    inline double lognormal(double mu, double sigma) { return( exp( mu + sigma * gsl_ran_ugaussian(m_rng) ) ); }
    inline double lognormal() { return( exp( gsl_ran_ugaussian(m_rng) ) ); }
};

RandomGenerator global_random_generator;
double random_uniform() { return global_random_generator.uniform(); }
double random_normal() { return global_random_generator.normal(); }
double random_lognormal() { return global_random_generator.lognormal(); }

void randul( double a, double b, double* u, int N, unsigned int seed )
{
    const gsl_rng_type*  t = gsl_rng_default;
    gsl_rng* r = gsl_rng_alloc(t);

    gsl_rng_set( r, seed );
    gsl_rng_env_setup();

    if( a > b )
        std::swap( a, b );

    double k = b - a;
    for( int n = 0; n < N; ++ n )
        u[n] = k * gsl_rng_uniform_pos( r ) + a;

    gsl_rng_free( r );
}

void randul(double a, double b, double* u, int N)
{
    unsigned int seed = 123;
    randul(a, b, u, N, seed);
}

void uniforml(double a, double b, double* u, int N)
{
    if( a > b )
        std::swap( a, b );

    double h = (b - a) / N;
    u[0] = h/2;
    for(int n=1; n<N; ++n)
        u[n] = u[n-1] + h;
}

}

#endif
