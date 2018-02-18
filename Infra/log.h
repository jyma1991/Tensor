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

#ifndef MARS_LOG_H
#define MARS_LOG_H 1

#define __mars_name(x)  #x
#define __mars_tostream(  stream, x ) stream <<COVAR<<#x<<codef<<"="<<COVAL<<std::setprecision(16)<< (x) <<codef<<std::endl
#define __mars_tostream2( stream, x ) stream <<COVAR<<#x<<codef<<"="<<COVAL<<std::setprecision(16)<< (x) <<codef<< " ";

#define __mars_print(  x ) __mars_tostream (std::cerr, x)
#define __mars_print2( x ) __mars_tostream2(std::cerr, x)

#if 0
#define __mars_log() std::cerr<< "Function " <<COFUNC << __FCN__  <<codef<<std::endl \
                    << "File " <<COFILE<< __FILE__ <<"\t"<< __LINE__ <<codef<<std::endl
//<<"\t"<< __TIME__ <<" "<< __DATE__
#endif

#define __mars_log() std::cerr<<COFILE<< __FILE__ <<":"<< __LINE__ <<" "<< COFUNC<< __FCN__ <<codef<< " " <<codef; fprintf( stderr, "\n" );

#define mars_log_code() __mars_log()
#define mars_log( x ) {__mars_print2( x ); __mars_log();}   // without {}, it is a bug
#define mars_log2( x, y ) {__mars_print2( x ); fprintf( stderr, ", " ); __mars_print2( y ); fprintf( stderr, "" ); }   // without {}, it is a bug

#define mars_log_variable( x )   __mars_print( x )
//#define logerr( x ) std::cerr<<coerr<< "Error" <<codef<<std::endl; Log( x );
#define mars_logwarn( warn, x ) std::cerr<<cowarn<< warn << ":" <<COVAR<< #x <<" = " <<COVAL<< x <<codef<<std::endl;
#define mars_logerror( x )  std::cerr<<coerr<< "Error" <<codef<<std::endl; \
                    mars_log( x ); \
                    std::cerr<<coerr<< "Exit"  <<codef<<std::endl; \
                    exit( EXIT_FAILURE )

//#define error() std::cerr<<coerr<< "Error" <<std::endl<<coerr<< "Exit"  <<codef<<std::endl; Log(); exit( EXIT_FAILURE )
#define exitline(x) std::cerr<<coerr<< "Exit"  <<codef<<std::endl; Log(x); exit( EXIT_FAILURE )

#define logmpid( rank ) printf( "rank = %s%d%s, PID = %s%d%s\n", COVAR, rank, codef, COVAR, getpid(), codef )
#define logpid() printf( "PID = %s%d%s\n", COVAR, getpid(), codef )

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

void _mars_log_time()
{
    std::chrono::time_point<std::chrono::system_clock> t;
    t = std::chrono::system_clock::now();
    t = std::chrono::time_point_cast<std::chrono::microseconds>(t);
    std::time_t time_t_x = std::chrono::system_clock::to_time_t(t);
    std::stringstream ssx;
    ssx << std::asctime(std::localtime(&time_t_x));
    std::string s = ssx.str();
    s.erase(s.size() - 1); // remove the line break
    fprintf(stderr, "%s%s%s\n", COFILE, s.c_str(), codef);
}

#define mars_log_time() _mars_log_time()

template<class T>
void print(T x)
{
    mars_log_variable(x);
}

//in   noimpl()
//{
//    std::cerr<<cowarn<< "Not implemented" <<codef<<std::endl;
//    __mars_log();
//    exit( EXIT_FAILURE );

//    return  0;
//}

long test_count = 0;

#define TEST_INFO(X) \
        { \
            std::string s = #X; \
            std::cerr<<COVAR<<"\nTEST: " << s << codef << "\n"; \
        }

//std::transform(s.begin(), s.end(), s.begin(), ::toupper);

#define TEST_EQ(X,Y) \
        { \
            std::cerr<< std::setw(6) << test_count << ": "; \
            if(X == Y) \
            { \
                std::cerr<<codef<< std::setprecision(16) << \
                        #X << "=" << (X) << ", " << \
                        #Y << "=" << (Y) << "\n"; \
            } \
            else \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X == Y ); \
            } \
            test_count ++;\
        }

#define ASSERT_EQ(X,Y) \
        { \
            if(X != Y) \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X == Y ); \
            } \
        }

#define ASSERT_NE(X,Y) \
        { \
            if(X == Y) \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X != Y ); \
            } \
        }

#define TEST_GT(X,Y) \
        { \
            std::cerr<< std::setw(6) << test_count << ": "; \
            if(X > Y) \
            { \
                std::cerr<<codef<< std::setprecision(16) << \
                        #X << "=" << (X) << ", " << \
                        #Y << "=" << (Y) << "\n"; \
            } \
            else \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X == Y ); \
            } \
            test_count ++;\
        }

#define ASSERT_GT(X,Y) \
        { \
            if( !(X > Y) ) \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X == Y ); \
            } \
        }

#define TEST_LT(X,Y) \
        { \
            std::cerr<< std::setw(6) << test_count << ": "; \
            if(X < Y) \
            { \
                std::cerr<<codef<< std::setprecision(16) << \
                        #X << "=" << (X) << ", " << \
                        #Y << "=" << (Y) << "\n"; \
            } \
            else \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X == Y ); \
            } \
            test_count ++;\
        }

#define ASSERT_LT(X,Y) \
        { \
            if( !(X < Y) ) \
            { \
                std::cerr<< std::setprecision(16) \
                        << COVAR << #X << "=" << COVAL << (X) << codef << ", " \
                        << COVAR << #Y << "=" << COVAL << (Y) << codef << " "; \
                __mars_log(); \
                assert( X < Y ); \
            } \
        }

#endif
