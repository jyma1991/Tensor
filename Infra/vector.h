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
#ifndef MARS_VECTOR_H
#define MARS_VECTOR_H 1

namespace mars{
namespace vector{

/*
template <class t>
vo  apply( t (*fp)( t ), t* l, ci& N )   //  fp Function Pointer
{
    for( ui i = 0; i < N; ++ i )
    {
        l[i] = (*fp)( l[i] );
    }
}
*/

/*
template<class T>
void mapcar( T (*fp)(T), T* l, const int& N )   //  fp Function Pointer
{
    for( unsigned int i = 0; i < N; ++ i )
    {
        l[i] = (*fp)( l[i] );
    }
}

template<class T0, class T1>
std::vector<T0> mapcar( T0 (*fp)(T1), std::vector<T1> A)
{
    std::vector<T0> B(A.size());
    int M = A.size();
    for( int i = 0; i < M; ++ i )
        B[i] = (*fp)( A[i] );

    return( B );
}
*/

template<class UnaryOperation, class T>
std::vector<T> mapcar( UnaryOperation OP, const std::vector<T>& A)
{
    int M = A.size();
    std::vector<T> B(M);
    for( int i = 0; i < M; ++ i )
        B[i] = OP( A[i] );
    return( B );
}

template<class UnaryOperation, class T1, class T2>
std::vector<T2> mapcar( UnaryOperation OP, const std::vector<T1>& A, T2 t2)
{
    int M = A.size();
    std::vector<T2> B(M);
    for( int i = 0; i < M; ++ i )
        B[i] = OP( A[i] );
    return( B );
}

template<class UnaryOperation, class T>
void mapcar_inplace( UnaryOperation OP, T* A, const int& M)
{
    for( int i = 0; i < M; ++ i )
        A[i] = OP( A[i] );
}

template<class UnaryOperation, class T>
void mapcar_inplace( UnaryOperation OP, std::vector<T>& A)
{
    int M = A.size();
    for( int i = 0; i < M; ++ i )
        A[i] = OP( A[i] );
}

template<class BinaryOperation, class T>
T reduce(BinaryOperation OP, const std::vector<T>& A, T rv)
{
    int M = A.size();
    for( int i = 0; i < M; ++ i )
        rv = OP(rv, A[i]);
    return( rv );
}

template<class BinaryOperation, class T0, class T1>
T1 reduce(BinaryOperation OP, T0* A, const int& M, T1 rv)
{
    for( int i = 0; i < M; ++ i )
        rv = OP(rv, A[i]);
    return( rv );
}

template<class T>
T sum(const std::vector<T>& v)
{
    T total(0);
    for(long i = 0; i < v.size(); i ++)
        total = T(total + v[i]);
    return total;
}

/*
template <class t>
vo  apply( t (*fp)( t ), t* l1, t* l2, ui N )
{
    for( ui i = 0; i < N; ++ i )
    {
        l2[i] = (*fp)( l1[i] );
    }
}

template <class t>
vo  apply( t (*fp)( t, t ), t  c, t* l1, t* l2, ui N )
{
    for( ui i = 0; i < N; ++ i )
    {
        l2[i] = (*fp)( l1[i], c );
    }
}
*/

template<class T>
std::vector<T> uniq(const std::vector<T>& v)
{
    std::vector<T> array;
    std::map<T, int> map;
    for(int i = 0; i < v.size(); i ++)
        if(map.find(v[i]) == map.end())
        {
            map[v[i]] = 0;
            array.push_back(v[i]);
        }
    return array;
}

template<typename t>
std::vector<t> grep( t* const x, int n, bool (*fp) (t) )
{
    std::vector<t>  v;
    typename std::vector<t>::iterator  it;
    for(int i=0; i<n; i++)
    {
        if(fp(x[i]))
            v.push_back( x[i] );
    }

    return  v;
}

template<typename T>
std::vector<T> slice(std::vector<T> v, mars::Slice slice)
{
    long begin = slice.begin();
    long end = slice.end();
    long stride = slice.step();

    std::vector<T> new_v;
    new_v.reserve( std::min( long(v.size()), slice.size() ) );
    for(long i = begin; i < end && i < v.size(); i += stride)
    {
        new_v.push_back( v[i] );
    }

    return new_v;
}

template<typename T>
void print(std::vector<T> v)
{
    for(size_t i = 0; i < v.size(); i ++)
        std::cerr<< v[i] << " ";
    std::cerr<< std::endl;
}

/*
struct  __le
{
    bool operator()(const double &a, const double &b)
    {
        return a <= b;
    }
};
*/

template<typename Pred>
std::vector<double> grep( double* const x, int n, Pred pred, double y )
{
    std::vector<double> v;
    typename std::vector<double>::iterator it;
    for(int i=0; i<n; ++i)
    {
        if( pred ( x[i], y ) )
            v.push_back( x[i] );
    }

    return  v;
}

template<typename Iterator, typename Pred, typename Operation>
void for_each_if(Iterator begin, Iterator end, Pred p, Operation op)
{
    for(; begin != end; ++begin)
    {
        if (p(*begin))
        {
            op(*begin);
        }
    }
}

template <class T>
T min(const std::vector<T>& l)
{
    T x = *min_element( l.begin(), l.end() );
    return  x;
}

template <class T>
T max(const std::vector<T>& l)
{
    T x = *max_element( l.begin(), l.end() );
    return  x;
}

template <class T>
std::vector<T> subvector(const std::vector<T>& V, mars::Slice S)
{
    S.set_end( std::min( S.end(), long(V.size()) ) );
    std::vector<T> X;
    //long j = 0;
    for(long i = S.begin(); i < S.end(); i ++)
    {
        X.push_back(V[i]);
        //fprintf(stderr, "%f\n", V[i]);
        //j ++;
    }
    return X;
}

template <class T>
T product(const std::vector<T>& l)
{
    T x;
    if( l.size() > 0 )
    {
        x = 1;
        for(long i = 0; i < l.size(); i ++)
            x *= l[i];
    }
    else
        x = 0;

    return  x;
}

#define testminv( l )  if( *min_element( l.begin(), l.end() ) <= 0 )    {   \
        std::cerr<<coerr<< "Min of vector" <<codef<<std::endl;  \
        mars_log( *min_element( l.begin(), l.end() ) );  \
        exit( 1 );  }

//  If I change db into template class t, it does noT work. Do noT know why
void scale(std::vector<double>* l, const double& unit)
{
    for(auto it = l->begin(); it != l->end(); ++ it )
        *it /= unit;
}

template <class T>
std::vector<T> join(const std::vector<T>& u, const std::vector<T>& v)
{
    std::vector<T> result = u;
    for(auto it = v.begin(); it != v.end(); it ++ )
        result.push_back(*it);
    return result;
}

template <class T>
std::vector<T> intersection(const std::vector<T>& A, const std::vector<T>& B)
{
    std::unordered_set<T> B_set(B.begin(), B.end());

    std::vector<T> C;
    C.reserve(A.size());

    for(auto x: A)
        if(B_set.find(x) != B_set.end())
            C.push_back(x);

    return C;
}

template<class T>
std::vector<T> diff(std::vector<T> A, std::vector<T> B)
{
    std::map<T,long> B_map;
    for(long i = 0; i < B.size(); i ++)
        B_map[B[i]] = i;

    std::vector<T> C;
    for(long i = 0; i < A.size(); i ++)
        if(B_map.find(A[i]) == B_map.end())
            C.push_back(A[i]);
    return( C );
}

/*
vo vector_from_file ( vector<db>& v, sT fn )
{
	ifs file;
	xl::open_file( file, fn );

    db  x;
	while ( !( file.eof() ) )
	{
		file >> x;
		v.push_back( x );
	}

    std::cerr<<cofunc<< __FCN__
        <<codef<< ": OK to read inpuT file: "
        <<cofile<< fn
        <<codef<< "\nInpuT size " << v.size() <<std::endl;

    file.close();
}
*/


//  write some convenience function for vectors
template<typename T>
class cvec
{
public:
    cvec(std::vector<T> v)
    {
        p = &v;
        std::cerr<<COWARN<< "#Create cvec" <<codef<<std::endl;
    }

    ~cvec()
    {
        std::cerr<<COWARN<< "#Terminate cvec" <<codef<<std::endl;
    }

    void show()
    {
        int n = p->size();
        for(int i=0; i<n; i++)
            std::cerr<< (*p)[i] << "\t";
        std::cerr<<std::endl;
    }

    void save( std::string fn )
    {
        file_from_vector(fn, *p);
    }

    void load( std::string fn )
    {
        vector_from_file(fn, *p);
    }

    T total()
    {
        return  total( *p );
    }

    T average()
    {
        return  total( *p ) / p->size();
    }

    T min()
    {
        return  min  ( *p );
    }

    T max()
    {
        return  max  ( *p );
    }

private:
    std::vector<T>*  p;  //  do noT wanT to manage mem
};

}}

#endif
