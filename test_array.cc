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

#include "mars.h"

int main()
{
	{
	    mars::String().set_printoptions<double>("%.0f");
    	TEST_INFO( One dimensional Array );
	    const long M = 10;
        mars::Array<double> A({ M });

        TEST_INFO( print array );
        A.print();

	    TEST_INFO( Assignment by element );

        for(long i = 0; i < M; i ++)
            A(i) = i;
        A.print();

        for(long i = 0; i < M; i ++)
            ASSERT_EQ(A(i), double( i ) );

        TEST_INFO( Slice: picking up the ones at 1-3-5... );
        A.slice(mars::Slice(1,10,2)).print();

        TEST_INFO( Slice: operate on the ones at 1-3-5... multiply by -1 );
        mars::Array<double> B = A.slice(mars::Slice(1,10,2));
        B *= -1;
        A.print();
    }

	{
    	TEST_INFO( One dimensional Array );
	    const long M = 10;
        mars::Array<double> A({ M });
        for(long i = 0; i < M; i ++)
            A(i) = i;
        A.print();

        TEST_INFO( operator+= );
        A += A;
        for(long i = 0; i < M; i ++)
            ASSERT_EQ(A(i), double( i*2 ) );
        A.print();

        mars::String().set_printoptions<double>("%.4f");
        TEST_INFO( 'map' in place: sin );
        A.map(sin);
        A.print();

        for(long i = 0; i < M; i ++)
            ASSERT_EQ(A(i), sin(double( i*2 )) );

        TEST_INFO( 'map' in place: floor at zero );
        A.map([](double x) -> double { return std::max(x, 0.0); });
        A.print();
        for(long i = 0; i < M; i ++)
            ASSERT_EQ(A(i), std::max(0.0, sin(double( i*2 ))) );
	}

	{
        mars::String().set_printoptions<double>("%.0f");
    	TEST_INFO( Two dimensional Array );
	    const long M = 10;
        mars::Array<double> A({ M, M });
        A.range(0, 1);
        TEST_INFO( Print array );
        A.print();
        TEST_INFO( Slice: elements at 1-1 1-3 1-5 ... 3-1 3-3 3-5 ... );
        A.slice(mars::Slice(1, M, 2), mars::Slice(1, M, 2)).print();
	}

	{
    	TEST_INFO( Three dimensional Array );
        mars::Array<double> A({ 2, 3, 5 });
        A.range(0, 1); // start from 1 with step 1
        TEST_INFO( Print array );
        A.print();
        TEST_INFO( Sum of array );
        TEST_EQ( A.sum(), 435.0);
        TEST_EQ( A.sum([](double x) -> double { return x; }), 435.0);

        TEST_INFO( Reshape array );
        A.reshape( {5, 6} );
        A.print();

        TEST_INFO( Reshape array back );
        A.reshape( {2, 3, 5} );
        A.print();
	}

	return 0;
}

