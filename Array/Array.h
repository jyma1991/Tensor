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

#ifndef MARS_TYPE_ARRAY_H
#define MARS_TYPE_ARRAY_H 1

#define DEBUG_MARS_ARRAY 1
#undef DEBUG_MARS_ARRAY

namespace mars{

template <typename T>
class Array;

template <typename T>
struct ThreadData
{
    Array<T>* array;
    long first;
    long last;
    T result;
};

template <typename T>
void* array_sum(void *p_i)
{
    //mars_log(pthread_self());
    mars::ThreadData<T>* p = (mars::ThreadData<T>*) p_i;
    typename mars::Array<T>::Iterator first = p->array->begin() + p->first;
    typename mars::Array<T>::Iterator last  = p->array->begin() + p->last;
    //mars_log( std::accumulate( first, last, T() ) );

    p->result = std::accumulate( first, last, T() );

    pthread_exit(NULL);
}

template <typename T>
class Array
{
private:
    std::vector<T> m_vec;
    T* m_p;
    bool m_own;
    bool m_phys;
    enum { MAX_RANK = 5 };

    long m_phys_size; // speed up
    long m_size; // speed up
    long m_rank;

    StackVector<long> m_phys_dims;
    StackVector<long> m_phys_strides;
    StackVector<long> m_log_strides;
    StackVector<long> m_log_shape;
    StackVector<Slice> m_log_slices;

private:
    inline void set_size()
    {
        m_phys_size = phys_shape().product();
        m_size      = shape().product();

        //mars_log( shape().to_string() );
        //print_config();

        //mars_log( m_phys_size );
        //mars_log( m_size );
    }

public:
    inline long rank() const { return m_phys_dims.size(); }

    void set_log_slices()
    {
        long rank = this->rank();
        m_log_slices.resize(rank);
        for(long i = 0; i < rank; i ++)
            m_log_slices[i] = Slice(0, m_phys_dims[i], 1);
    }

    inline void set_log_shape()
    {
        //mars_log(m_log_slices[0].to_string());
        //mars_log(m_log_slices[0].size());
        if(m_phys)
            m_log_shape = m_phys_dims;
        else
        {
            long rank = this->rank();
            StackVector<long> A(rank);
            for(long i = 0; i < rank; i ++)
                A[i] = m_log_slices[i].size();
            m_log_shape = A;
        }
    }

    inline void set_strides()
    {
        long rank = this->rank();
        for(long j = 0; j < rank; j ++)
            m_log_slices[j].set_end( std::min( m_log_slices[j].end(), phys_shape()[j] ) );

        m_phys_strides.resize(0);
        m_log_strides.resize(0);

        long j;
        long i;

        for(j = 0; j < rank; j ++)
        {
            long total_size = 1;
            for(i = j+1; i < rank; i ++)
                total_size *= m_phys_dims[i];
            m_phys_strides.push_back(total_size);
        }

        for(j = 0; j < rank; j ++)
        {
            long total_size = 1;
            for(i = j+1; i < rank; i ++)
                total_size *= m_log_slices[i].size();
            m_log_strides.push_back(total_size);
        }

        set_log_shape();
        set_size();
    }

    inline StackVector<long> phys_shape() const { return m_phys_dims; }
    inline StackVector<long> shape() const { return m_log_shape; }

    inline StackVector<long> phys_strides() const { return m_phys_strides; }
    inline StackVector<long> log_strides()  const { return m_log_strides; }

    inline long phys_size() const { return m_phys_size; }
    inline long size()      const { return m_size; }

    /*
    inline StackVector<long> to_index(long i) const
    {
        StackVector<long> indices(m_phys_strides.size());
        long remainder = i;
        for(long j = 0; j < indices.size(); j ++)
        {
            indices[j] = std::floor(remainder / strides[j]);
            remainder = remainder % strides[j];
        }
        return indices;
    }
    */

private:
    bool owner() const { return m_own; }
    bool is_phys() const { return m_phys; }

    void sync_p()
    {
        if(owner())
            m_p = m_vec.size() ? &(m_vec[0]) : nullptr;
    }

    template<typename InputIndex>
    void create_from_vec(const StackVector<InputIndex>& dims_i, const T& x_i) // own memory
    {
        StackVector<long> dims(dims_i.begin(), dims_i.end());
        m_own = true;
        m_phys = true;
        m_phys_dims = dims;
        set_log_slices();
        set_strides();
        m_vec = std::vector<T>(size(), x_i);
        sync_p();
        log_constructor();
    }

    template<typename InputIndex>
    void create_from_pointer(T* p_i, const StackVector<InputIndex> dims_i) // does NOT own memory
    {
        StackVector<long> dims(dims_i.begin(), dims_i.end());
        m_own = false;
        m_phys = true;
        m_phys_dims = dims;
        set_log_slices();
        set_strides();
        m_p = p_i;
        log_constructor();
    }

    void copy_except_vec(const Array<T>& rhs)
    {
        m_p = rhs.m_p;
        m_own = rhs.m_own;
        m_phys_size = rhs.m_phys_size;
        m_size = rhs.m_size;

        m_phys = rhs.m_phys;
        m_phys_dims = rhs.m_phys_dims;
        m_log_slices = rhs.m_log_slices;
        m_log_shape = rhs.m_log_shape;
        set_strides();
    }

    void create_from_assignment(const Array<T>& rhs)
    {
        copy_except_vec(rhs);
        m_vec = rhs.m_vec;
        sync_p();
        log_constructor();
    }

    void create_from_move(Array<T>& rhs)
    {
#ifdef DEBUG_MARS_ARRAY
        fprintf(stderr, "Move constructor\n" );
#endif
        copy_except_vec(rhs);

#ifdef DEBUG_MARS_ARRAY
        fprintf(stderr, "&(rhs.m_vec[0])=%p\n", &(rhs.m_vec[0]));
#endif
        m_vec = std::move(rhs.m_vec);

#ifdef DEBUG_MARS_ARRAY
        fprintf(stderr, "&(m_vec[0])=%p\n", &(m_vec[0]));
#endif
        sync_p();
        log_constructor();
    }

    void check_bounds(long array_index) const
    {
        /* mars_log_variable(i); mars_log_variable(size()); */
        assert( array_index >= 0 );
        if( array_index >= phys_size() )
        {
            fprintf(stderr, "array_index=%ld\n", array_index );
            fprintf(stderr, "phys_size()=%ld\n", phys_size() );
            ASSERT_LT( array_index, phys_size() );
        }
    }

    void check_rank(long array_dim) const { ASSERT_EQ( array_dim, rank() ); }
    void check_1d() const { check_rank(1); }

public:
    explicit Array() { create_from_vec(StackVector<long>{}, T()); }

    template<typename InputIndex>
    explicit Array(const StackVector<InputIndex>& dims_i, const T& x_i) { create_from_vec(dims_i, x_i); }

    template<typename InputIndex>
    explicit Array(const std::initializer_list<InputIndex>& dims_i, const T& x_i) { create_from_vec(StackVector<InputIndex>(dims_i), x_i); }

    template<typename InputIndex>
    explicit Array(const StackVector<InputIndex>& dims_i) { create_from_vec(dims_i, T()); }

    template<typename InputIndex>
    explicit Array(const std::initializer_list<InputIndex>& dims_i) { create_from_vec(StackVector<InputIndex>(dims_i), T()); }

    //explicit Array(long N_i) { create_from_vec( { N_i }, T()); }
    //explicit Array(long N_i, const T& x_i) { create_from_vec( { N_i }, x_i); }

    template<typename InputIndex>
    explicit Array(T* p, const StackVector<InputIndex>& dims_i) { create_from_pointer(p, dims_i); }

    template<typename InputIndex>
    explicit Array(T* p, const std::initializer_list<InputIndex>& dims_i) { create_from_pointer(p, StackVector<InputIndex>( dims_i )); }

    Array(const Array<T>& rhs) { create_from_assignment( rhs ); }
    Array(Array&& rhs) { create_from_move(rhs); }

    Array& operator=( const Array<T>& rhs )
    {
        create_from_assignment( rhs );
        return *this;
    }

    void set_value(const Array<T>& rhs) // this is preferred for value assignment
    {
        ASSERT_EQ( this->shape(), rhs.shape() );
        Iterator i = this->begin();
        CIterator j = rhs.cbegin();
        for(; i != this->end(); ++ i)
        {
            *i = *j;
            j ++;
        }
    }

    void check_owner() const { assert( m_own == true ); }
    void check_phys() const { assert( m_phys == true ); }
    inline T* address() const { return m_p; }
    inline T* address() { return m_p; }

    // without this input k, it will be a bug.
    inline StackVector<long> to_indices(long k) const
    {
        const StackVector<long>& array_strides = m_log_strides;
        StackVector<long> array_indices( rank() );
        for(long i = 0; i < array_strides.size(); i ++)
        {
            if(k < INT_MAX && array_strides[i] < INT_MAX )
            {
                array_indices[i] = (unsigned int)(k) / (unsigned int)(array_strides[i]);
                k                = (unsigned int)(k) % (unsigned int)(array_strides[i]);
            }
            else
            {
                array_indices[i] = k / array_strides[i];
                k                = k % array_strides[i];
            }
        }
        return array_indices;
    }

    template<bool is_const>
    class GeneralIterator: public std::iterator<std::random_access_iterator_tag, T, long, const T*, const T&>
    {
    typedef typename std::conditional<is_const, const Array&, Array&>::type data_type;

    private:
        data_type m_data;
        long index;
        bool m_is_contiguous;
        T* m_begin_ptr;

        inline void set_is_contiguous()
        {
            long size = m_data.size();
            m_begin_ptr = &( m_data.operator()( m_data.to_indices(0) ) );
            T* last_ptr = &( m_data.operator()( m_data.to_indices( size-1 ) ) );
            if( std::distance( m_begin_ptr, last_ptr ) == size-1 )
                m_is_contiguous = true;
            else
                m_is_contiguous = false;
        }

        inline T* begin_ptr() const { return m_begin_ptr; }

    public:
        typedef GeneralIterator self_type;
        typedef T value_type;
        typedef T& reference;
        typedef T* pointer;
        typedef std::forward_iterator_tag iterator_category;
        typedef long difference_type;

        GeneralIterator(data_type data, long index_input) : m_data{ data }, index{ index_input } { set_is_contiguous(); }
        GeneralIterator(const GeneralIterator& rhs) : m_data( rhs.m_data )
        {
            index = rhs.index;
            m_is_contiguous = rhs.m_is_contiguous;
            m_begin_ptr = rhs.m_begin_ptr;
        }

        GeneralIterator& operator=(const GeneralIterator& rhs)
        {
            index = rhs.index;
            m_is_contiguous = rhs.m_is_contiguous;
            m_begin_ptr = rhs.m_begin_ptr;
            return *this;
        }

        inline bool is_contiguous() const { return m_is_contiguous; }

        bool operator==(const GeneralIterator& rhs) const { return this->index == rhs.index; }
        bool operator!=(const GeneralIterator& rhs) const { return this->index != rhs.index; }
        friend long operator -(const GeneralIterator& lhs, const GeneralIterator& rhs) { return lhs.index - rhs.index; }
        friend bool operator <(const GeneralIterator& lhs, const GeneralIterator& rhs) { return lhs.index < rhs.index; }
        friend bool operator >(const GeneralIterator& lhs, const GeneralIterator& rhs) { return lhs.index > rhs.index; }
        friend bool operator<=(const GeneralIterator& lhs, const GeneralIterator& rhs) { return lhs.index <= rhs.index; }
        friend bool operator>=(const GeneralIterator& lhs, const GeneralIterator& rhs) { return lhs.index >= rhs.index; }
        friend GeneralIterator operator+(const GeneralIterator& lhs, const long& rhs) { GeneralIterator it(lhs); it.index += rhs; return it; }
        friend GeneralIterator operator-(const GeneralIterator& lhs, const long& rhs) { GeneralIterator it(lhs); it.index -= rhs; return it; }

        GeneralIterator& operator++()    { ++ index; return *this; }
        GeneralIterator& operator--()    { /*raise(SIGSEGV); mars_log(index);*/ -- index; /*mars_log(index);*/ assert(index>=0); return *this; }
        GeneralIterator  operator++(int) { GeneralIterator tmp(*this); index ++; return tmp; }
        GeneralIterator  operator--(int) { GeneralIterator tmp(*this); index --; return tmp; }
        GeneralIterator& operator+=(long d) { index += d; mars_log(index); return *this; }
        GeneralIterator& operator-=(long d) { index -= d; mars_log(index); return *this; }
        inline long size() { return m_data.size(); }

        inline reference operator*() const
        {
            assert(index >= 0);
            assert(index <= size());

            if( is_contiguous() )
            {
                return *(begin_ptr() + index);
            }
            else
            {
                StackVector<long> array_indices = m_data.to_indices(index);
                return m_data.operator()( array_indices );
            }
        }

        inline reference operator*()
        {
            assert(index >= 0);
            assert(index <= size());

            if( is_contiguous() )
            {
                return *(begin_ptr() + index);
            }
            else
            {
                StackVector<long> array_indices = m_data.to_indices(index);
                return m_data.operator()( array_indices );
            }
        }

        inline pointer operator->()
        {
            assert(index >= 0);
            assert(index <= size());
            StackVector<long> array_indices = m_data.to_indices(index);
            return &( m_data.operator()( array_indices ) );
        }
    };

    typedef GeneralIterator<true>  CIterator;
    typedef GeneralIterator<false> Iterator;

    CIterator cbegin() const { return CIterator(*this, 0); }
    CIterator cend()   const { return CIterator(*this, size()); }

    Iterator begin(){ return Iterator(*this, 0); }
    Iterator end()  { return Iterator(*this, size()); }

    void sort()
    {
        std::sort(begin(), end(),
            [this](const T& a, const T& b) -> bool
            { return (a < b); } );
    }

    void resize_sync_param()
    {
        m_phys_dims[0] = m_vec.size();
        m_log_slices[0].set_end(m_vec.size());
    }

    void push_back(const T& x_i) { check_owner(); check_1d(); m_vec.push_back(x_i); resize_sync_param(); sync_p(); }
    void reserve(long M)         { check_owner(); check_1d(); m_vec.reserve(M);     resize_sync_param(); sync_p(); }
    void resize(long M)          { check_owner(); check_1d(); m_vec.resize(M);      resize_sync_param(); sync_p(); }

    ~Array() { log_destructor(); }

    StackVector<Slice> slices_to_vector_impl() { return StackVector<Slice>(); }

    template<typename First, typename... Rest>
    StackVector<Slice> slices_to_vector_impl(First first, Rest... rest)
    {
        StackVector<Slice> v;
        v.push_back(first);
        StackVector<Slice> new_v = slices_to_vector_impl(rest...);
        for(long i = 0; i < new_v.size(); i ++)
            v.push_back( new_v[i] );
        return v;
    }

    template<typename First, typename... Rest>
    Array<T> slice(First first, Rest... rest)
    {
        StackVector<Slice> v = slices_to_vector_impl(first, rest...);
        check_phys();
        check_rank(v.size());
        Array<T> C(nullptr, {0});
        C.copy_except_vec(*this);
        C.m_own = false;
        C.m_phys = false;
        C.m_log_slices = v;
        C.set_strides();

        return C;
    }

    Array<T> slice_vec(std::vector<Slice> v)
    {
        switch(rank())
        {
        case 1: return slice( v[0] );
        case 2: return slice( v[0], v[1] );
        case 3: return slice( v[0], v[1], v[2] );
        case 4: return slice( v[0], v[1], v[2], v[3] );
        case 5: return slice( v[0], v[1], v[2], v[3], v[4] );
        default: mars_log( rank() ); assert( rank() < 6 );
        }
        return Array<T>();
    }

    Array<T> subarray(StackVector<long> leading_indices) // return a deep copy
    {
        std::vector<Slice> v;
        StackVector<long> dims;
        for(long i = 0; i < rank(); i ++)
        {
            if(i < leading_indices.size())
                v.push_back( Slice( leading_indices[i], leading_indices[i]+1, 1 ) );
            else
            {
                v.push_back( m_log_slices[i] );
                dims.push_back( m_phys_dims[i] );
            }
            //mars_log( v[i].to_string() );
        }

        //slice_vec( v ).print();

        Array<T> A = slice_vec( v ).copy();
        //A.print("subarray");
        A.reshape(dims);
        return A;
    }

    Array& reshape(StackVector<long> new_phys_dims)
    {
        check_owner();
        check_phys();
        assert(size() == new_phys_dims.product());
        m_phys_dims = new_phys_dims;
        set_log_slices();
        set_strides();
        return *this;
    }

    void transpose(StackVector<long> new_axes)
    {
        StackVector<long> axes;
        for(long i = 0; i < rank(); i ++)
            axes.push_back(i);
        assert( axes.size() == new_axes.size() );
        assert( std::set<long>(axes.begin(), axes.end()) == std::set<long>(new_axes.begin(), new_axes.end()) );

        Array C(*this);
        const long N = C.size();
        const long rank = C.rank();
        StackVector<long> shape = C.shape();

        StackVector<long> new_shape( rank );
        for(long i = 0; i < rank; i ++)
            new_shape[i] = shape[ new_axes[i] ];
        reshape( new_shape );

        StackVector<long> indices;
        StackVector<long> new_indices( rank );
        for(long i = 0; i < N; i ++)
        {
            indices = C.to_indices(i);
            for(long k = 0; k < rank; k ++)
                new_indices[k] = indices[ new_axes[k] ];
            operator()(new_indices) = C.operator()(indices);
        }
    }

    void transpose()
    {
        { check_rank(2); }
        StackVector<long> new_axes{1,0};
        transpose( new_axes );
    }

    Array<Array<T> > split(StackVector<long> axes_i)
    {
        StackVector<long> axes;
        for(long i = 0; i < rank(); i ++)
            axes.push_back(i);

        {
            check_owner();
            check_phys();
            assert( std::is_sorted( axes_i.begin(), axes_i.end() ) );
            assert( std::includes(axes.begin(), axes.end(), axes_i.begin(), axes_i.end()) );
        }

        std::vector<long> pivot_axes_vec;
        std::set_difference(axes.begin(), axes.end(), axes_i.begin(), axes_i.end(),
                std::inserter(pivot_axes_vec, pivot_axes_vec.begin()));

        StackVector<long> pivot_axes(&(pivot_axes_vec[0]), &(pivot_axes_vec[0]) + pivot_axes_vec.size());

        std::vector<std::vector<Slice> > vs;

        std::unordered_set<std::string> set;
        for(long i = 0; i < size(); i ++)
        {
            StackVector<long> indices = to_indices(i);
            //mars_log(indices.size());

            std::vector<Slice> slices( indices.size(), Slice() );
            //mars_log_code();

            std::string key;
            for(long k = 0; k < indices.size(); k ++)
            {
                if( std::find(axes_i.begin(), axes_i.end(), k) == axes_i.end() )
                    slices[k] = Slice(0, shape()[k], 1);
                else
                    slices[k] = Slice(indices[k], indices[k]+1, 1);
                key = key + String().sprintf("(%ld,%ld,%ld),", slices[k].begin(), slices[k].end(), slices[k].step());
            }

            if(set.find(key) == set.end())
            {
                set.insert(key);
                vs.push_back( slices );
            }
        }

        Array<Array<T> > splits( { long( vs.size() ) } );
        for(long i = 0; i < vs.size(); i ++)
            splits( i ) = this->slice_vec( vs[i] );
        return splits;
    }

    class ForEach
    {
    private:
        long i0, i1, i2, i3, i4, i5;
        Array<T>& m_A;

        void init()
        {   i0 = i1 = i2 = i3 = i4 = i5 = 0; }

    public:
        ForEach(Array<T>& A_i) : m_A(A_i) {}

        T* begin()
        {
            init();
            next(); // without this, it will be a bug corrupting memory
            switch(m_A.rank())
            {
            case 1: return &(m_A.operator()(0));
            case 2: return &(m_A.operator()(0, 0));
            case 3: return &(m_A.operator()(0, 0, 0));
            case 4: return &(m_A.operator()(0, 0, 0, 0));
            case 5: return &(m_A.operator()(0, 0, 0, 0, 0));
            default: mars_log( m_A.rank() ); assert( m_A.rank() < 6 );
            }
            return nullptr;
        }

        T* next()
        {
            StackVector<long> shape = m_A.shape();
            switch(m_A.rank())
            {
            case 1:
                while(i0 < shape[0])
                {
                    T* x = &(m_A.operator()(i0));
                    i0 ++;
                    return x;
                }
                return nullptr;
            case 2:
                while(i0 < shape[0])
                while(i1 < shape[1])
                {
                    T* x = &(m_A.operator()(i0,i1));
                    i1 ++;
                    if(i1 == shape[1]) { i1 = 0; i0 ++; }
                    return x;
                }
                return nullptr;
            case 3:
                while(i0 < shape[0])
                while(i1 < shape[1])
                while(i2 < shape[2])
                {
                    T* x = &(m_A.operator()(i0,i1,i2));
                    i2 ++;
                    if(i2 == shape[2]) { i2 = 0; i1 ++; } // carrying of arithmetics
                    if(i1 == shape[1]) { i1 = 0; i0 ++; }
                    return x;
                }
                return nullptr;
            case 4:
                while(i0 < shape[0])
                while(i1 < shape[1])
                while(i2 < shape[2])
                while(i3 < shape[3])
                {
                    T* x = &(m_A.operator()(i0,i1,i2));
                    i3 ++;
                    if(i3 == shape[3]) { i3 = 0; i2 ++; }
                    if(i2 == shape[2]) { i2 = 0; i1 ++; }
                    if(i1 == shape[1]) { i1 = 0; i0 ++; }
                    return x;
                }
                return nullptr;
            case 5:
                while(i0 < shape[0])
                while(i1 < shape[1])
                while(i2 < shape[2])
                while(i3 < shape[3])
                while(i4 < shape[4])
                {
                    T* x = &(m_A.operator()(i0,i1,i2));
                    i4 ++;
                    if(i4 == shape[4]) { i4 = 0; i3 ++; }
                    if(i3 == shape[3]) { i3 = 0; i2 ++; }
                    if(i2 == shape[2]) { i2 = 0; i1 ++; }
                    if(i1 == shape[1]) { i1 = 0; i0 ++; }
                    return x;
                }
                return nullptr;
            default: mars_log( m_A.rank() ); assert( m_A.rank() < 6 );
            }
            return nullptr;
        }
    };

    inline T& operator[] (long i) { return *Iterator(*this, i); }
    inline const T& operator[](long i) const  { return *Iterator(*this, i); }

    Array<T> copy() // always deep copy
    {
        Array<T> A( shape() );
        long j = 0;
        for(Iterator it = begin(); it != end(); ++ it)
        {
            A.m_p[j] = *it;
            j ++;
        }
        return( A );
    }

    void concat(Array<T>& rhs)
    {
        check_owner();
        check_phys();
        assert( m_phys_strides == rhs.m_log_strides );
        long M = rhs.size();
        for(long i = 0; i < M; i ++)
            m_vec.push_back(rhs[i]);
        sync_p();
    }

    inline T& operator() (long n_0) const
    {
        long i;
        if(m_phys)
            i = n_0 * m_phys_strides[0];
        else
            i = (m_log_slices[0].begin() + m_log_slices[0].step() * n_0) * m_phys_strides[0];
#ifdef DEBUG_MARS_ARRAY
        check_rank(1);
        check_bounds(i);
#endif
        return m_p[i];
    }

    inline T& operator() (long n_0)
    {   return const_cast<T&>(static_cast<const Array<T>*>(this) -> operator()(n_0)); }

    inline T& operator() (long n_0, long n_1) const
    {
        long i;
        if(m_phys)
            i = n_0 * m_phys_strides[0] +
                n_1 * m_phys_strides[1];
        else
            i = (m_log_slices[0].begin() + m_log_slices[0].step() * n_0) * m_phys_strides[0] +
                (m_log_slices[1].begin() + m_log_slices[1].step() * n_1) * m_phys_strides[1];

#ifdef DEBUG_MARS_ARRAY
        check_rank(2);
        check_bounds(i);
        ASSERT_LT( n_0, m_log_slices[0].size() );
        ASSERT_LT( n_1, m_log_slices[1].size() );
#endif
        return m_p[i];
    }

    inline T& operator() (long n_0, long n_1)
    {   return const_cast<T&>(static_cast<const Array<T>*>(this) -> operator()(n_0, n_1)); }

    inline T& operator() (long n_0, long n_1, long n_2) const
    {
        long i;
        if(m_phys)
            i = n_0 * m_phys_strides[0] +
                n_1 * m_phys_strides[1] +
                n_2 * m_phys_strides[2];
        else
            i = (m_log_slices[0].begin() + m_log_slices[0].step() * n_0) * m_phys_strides[0] +
                (m_log_slices[1].begin() + m_log_slices[1].step() * n_1) * m_phys_strides[1] +
                (m_log_slices[2].begin() + m_log_slices[2].step() * n_2) * m_phys_strides[2];

#ifdef DEBUG_MARS_ARRAY
        check_rank(3);
        check_bounds(i);
        ASSERT_LT( n_0, m_log_slices[0].size() );
        ASSERT_LT( n_1, m_log_slices[1].size() );
        ASSERT_LT( n_2, m_log_slices[2].size() );
#endif
        return m_p[i];
    }

    inline T& operator() (long n_0, long n_1, long n_2)
    {   return const_cast<T&>(static_cast<const Array<T>*>(this) -> operator()(n_0, n_1, n_2)); }

    inline T& operator() (long n_0, long n_1, long n_2, long n_3) const
    {
        long i;
        if(m_phys)
            i = n_0 * m_phys_strides[0] +
                n_1 * m_phys_strides[1] +
                n_2 * m_phys_strides[2] +
                n_3 * m_phys_strides[3];
        else
            i = (m_log_slices[0].begin() + m_log_slices[0].step() * n_0) * m_phys_strides[0] +
                (m_log_slices[1].begin() + m_log_slices[1].step() * n_1) * m_phys_strides[1] +
                (m_log_slices[2].begin() + m_log_slices[2].step() * n_2) * m_phys_strides[2] +
                (m_log_slices[3].begin() + m_log_slices[3].step() * n_3) * m_phys_strides[3];

#ifdef DEBUG_MARS_ARRAY
        check_rank(4);
        check_bounds(i);
        ASSERT_LT( n_0, m_log_slices[0].size() );
        ASSERT_LT( n_1, m_log_slices[1].size() );
        ASSERT_LT( n_2, m_log_slices[2].size() );
        ASSERT_LT( n_3, m_log_slices[3].size() );
#endif
        return m_p[i];
    }

    inline T& operator() (long n_0, long n_1, long n_2, long n_3)
    {   return const_cast<T&>(static_cast<const Array<T>*>(this) -> operator()(n_0, n_1, n_2, n_3)); }

    inline T& operator() (long n_0, long n_1, long n_2, long n_3, long n_4) const
    {
        long i;
        if(m_phys)
            i = n_0 * m_phys_strides[0] +
                n_1 * m_phys_strides[1] +
                n_2 * m_phys_strides[2] +
                n_3 * m_phys_strides[3] +
                n_4 * m_phys_strides[4];
        else
            i = (m_log_slices[0].begin() + m_log_slices[0].step() * n_0) * m_phys_strides[0] +
                (m_log_slices[1].begin() + m_log_slices[1].step() * n_1) * m_phys_strides[1] +
                (m_log_slices[2].begin() + m_log_slices[2].step() * n_2) * m_phys_strides[2] +
                (m_log_slices[3].begin() + m_log_slices[3].step() * n_3) * m_phys_strides[3] +
                (m_log_slices[4].begin() + m_log_slices[4].step() * n_4) * m_phys_strides[4];

#ifdef DEBUG_MARS_ARRAY
        check_rank(5);
        check_bounds(i);
        ASSERT_LT( n_0, m_log_slices[0].size() );
        ASSERT_LT( n_1, m_log_slices[1].size() );
        ASSERT_LT( n_2, m_log_slices[2].size() );
        ASSERT_LT( n_3, m_log_slices[3].size() );
        ASSERT_LT( n_4, m_log_slices[4].size() );
#endif
        return m_p[i];
    }

    inline T& operator() (long n_0, long n_1, long n_2, long n_3, long n_4)
    {   return const_cast<T&>(static_cast<const Array<T>*>(this) -> operator()(n_0, n_1, n_2, n_3, n_4)); }

    inline T& operator() (const StackVector<long>& ns) const
    {
        switch(ns.size())
        {
            case 1: return operator()(ns[0]);
            case 2: return operator()(ns[0], ns[1]);
            case 3: return operator()(ns[0], ns[1], ns[2]);
            case 4: return operator()(ns[0], ns[1], ns[2], ns[3]);
            case 5: return operator()(ns[0], ns[1], ns[2], ns[3], ns[4]);
            case 0:  mars_log(ns.size()); std::cerr<<COERR<< "Error: Array indices of operator() is empty\n"; assert( ns.size() != 0 );
            default: mars_log(ns.size()); std::cerr<<COERR<< "Error: Array indices of operator() too large in dimension\n"; assert( ns.size() < 6 );
        }

        // to fix compilation warning, will never reach here
        return operator()(ns[0]);
    }

    void print_shape(std::string name="Array")
    {
        fprintf(stderr, "%s%s%s%s:\n", COVAR, name.c_str(), mars::String().to_string( shape() ).c_str(), codef);
    }

    void print_config(std::string name="Array")
    {
        String _;
        _.set_delim(",");
        _.set_bracket("(", ")");

        std::vector<std::vector<std::string> > table {
            { "name", _.to_string(name) },
            { "m_own", _.to_string(m_own) },
            { "m_phys", _.to_string(m_phys) },
            { "m_p", _.sprintf("%p", m_p) },
            { "m_phys_dims", _.to_string(m_phys_dims) },
            { "m_phys_strides", _.to_string(m_phys_strides) },
            { "m_log_strides", _.to_string(m_log_strides) } };

        std::string str = "(";
        for(long i = 0; i < rank(); i ++)
            str = str + String().sprintf("(%ld,%ld,%ld)%s", m_log_slices[i].begin(), m_log_slices[i].end(), m_log_slices[i].step(), i == rank()-1 ? "" : ",");
        str = str + ")";
        table.push_back( { "m_log_slices", str } );

        std::vector<std::string> v0, v1;
        for(auto& x : table)
        {
            v0.push_back(x[0]);
            v1.push_back(x[1]);
        }
        std::vector<std::vector<std::string> > tab{ v0, v1 };
        String().prettyprint_table(tab, false, false);
    }

    void print(std::string name, StackVector<long> N_print)
    {
        //T x;
        //std::string format = mars::cpp::printf_format(x) + " ";
        if(rank())
            fprintf(stderr, "%s%s%s%s:\n", COVAR, name.c_str(), mars::String().to_string(shape()).c_str(), codef);
        else
            fprintf(stderr, "%s%s(0)%s\n", COVAR, name.c_str(), codef);

#ifdef DEBUG_MARS_ARRAY
        print_config(name);
#endif

        if(rank() == 1)
        {
            /*
            for(long i = 0; i < limit; i ++)
            {
                fprintf(stderr, "%s ", mars::infra::string::to_string( operator()(i) ).c_str() );
                if((i+1) % 10 == 0 && i != limit - 1)
                    fprintf(stderr, "\n");
            }
            fprintf(stderr, "\n");
            */

            std::vector<std::vector<T> > V;
            for(long i = 0; i < std::min(N_print[0], shape()[0]); i ++)
            {
                if(i % 10 == 0)
                    V.push_back( std::vector<T>() );
                V[ V.size()-1 ].push_back( operator()(i) );
            }
            mars::String().prettyprint_table(V, true);
        }
        else if(rank() == 2)
        {
            /*
            for(long i = 0; i < std::min(N_print[0], shape()[0]); i ++)
            {
                for(long j = 0; j < std::min(N_print[1], shape()[1]); j ++)
                    fprintf(stderr, "%s ", mars::infra::string::to_string( operator()(i, j) ).c_str() );
                fprintf(stderr, "\n");
            }
            */

            std::vector<std::vector<T> > V;

            for(long i = 0; i < std::min(N_print[0], shape()[0]); i ++)
            {
                V.push_back( std::vector<T>() );
                for(long j = 0; j < std::min(N_print[1], shape()[1]); j ++)
                    V[i].push_back( operator()(i, j) );
            }
            mars::String().prettyprint_table(V, true);
        }
        else if(rank() == 3)
        {
            for(long k = 0; k < std::min(N_print[0], shape()[0]); k ++)
            {
                fprintf(stderr, "[%ld]:\n", k);
                /*
                for(long i = 0; i < std::min(N_print[1], shape()[1]); i ++)
                {
                    for(long j = 0; j < std::min(N_print[2], shape()[2]); j ++)
                        fprintf(stderr, "%s ", mars::infra::string::to_string( operator()(k, i, j) ).c_str()  );
                    fprintf(stderr, "\n");
                }
                */

                std::vector<std::vector<T> > V;
                for(long i = 0; i < std::min(N_print[1], shape()[1]); i ++)
                {
                    //mars_log( std::min(N_print[1], shape()[1]) );
                    //exit(1);

                    V.push_back( std::vector<T>() );
                    for(long j = 0; j < std::min(N_print[2], shape()[2]); j ++)
                        V[i].push_back( operator()(k, i, j) );
                }
                mars::String().prettyprint_table(V, true);
            }
        }
        else if(rank() == 4)
        {
            for(long k0 = 0; k0 < std::min(N_print[0], shape()[0]); k0 ++)
            for(long k1 = 0; k1 < std::min(N_print[1], shape()[1]); k1 ++)
            {
                /*
                fprintf(stderr, "[%d,%d]:\n", k0, k1);
                for(long i = 0; i < std::min(N_print[2], shape()[2]); i ++)
                {
                    for(long j = 0; j < std::min(N_print[3], shape()[3]); j ++)
                        fprintf(stderr, "%s ",
                                mars::infra::string::to_string( operator()(k0, k1, i, j) ).c_str()  );
                    fprintf(stderr, "\n");
                }
                */

                fprintf(stderr, "[%ld,%ld]:\n", k0, k1);
                std::vector<std::vector<T> > V;
                for(long i = 0; i < std::min(N_print[2], shape()[2]); i ++)
                {
                    V.push_back( std::vector<T>() );
                    for(long j = 0; j < std::min(N_print[3], shape()[3]); j ++)
                        V[i].push_back( operator()(k0, k1, i, j)  );
                }
                mars::String().prettyprint_table(V, true);
            }
        }
        else if(rank() == 5)
        {
            fprintf(stderr, "%s%s(%ld,%ld,%ld,%ld,%ld)%s:\n", COVAR, name.c_str(),
                   shape()[0], shape()[1], shape()[2], shape()[3], shape()[4], codef);
            for(long k0 = 0; k0 < std::min(N_print[0], shape()[0]); k0 ++)
            for(long k1 = 0; k1 < std::min(N_print[1], shape()[1]); k1 ++)
            for(long k2 = 0; k2 < std::min(N_print[2], shape()[2]); k2 ++)
            {
                fprintf(stderr, "[%ld,%ld,%ld]:\n", k0, k1, k2);
                std::vector<std::vector<T> > V;
                for(long i = 0; i < std::min(N_print[3], shape()[3]); i ++)
                {
                    V.push_back( std::vector<T>() );
                    for(long j = 0; j < std::min(N_print[4], shape()[4]); j ++)
                        V[i].push_back( operator()(k0, k1, k2, i, j) );
                }
                mars::String().prettyprint_table(V, true);
            }
        }
        else
        {
            //mars_log_code();

            assert(shape().size() <= 5);
        }
    }

    void print(std::string name) { print(name + ": Array", shape()); }
    void print() { print("Array", shape()); }

    void range(T a, T inc)
    {
        long i = 0;
        for(Iterator it = begin(); it != end(); it ++)
        {
            *it = a + T( i ) * inc;
            i ++;
        }
    }

    void random_normal(RandomGenerator& rng)
    {
        for(Iterator it = begin(); it != end(); it ++)
        {
            //mars_log(*it);
            *it = rng.normal();
        }
    }

    void uniq()
    {
        check_owner();
        check_1d();
        std::vector<T> v( begin(), end() );
        typename std::vector<T>::iterator it = std::unique(v.begin(), v.end());   // 10 20 30 20 10 ?  ?  ?  ?
        v.resize( std::distance(v.begin(), it) ); // 10 20 30 20 10
        m_vec = v;
        resize(m_vec.size());
    }

    std::string strjoin(const std::string& delim=",")
    {
        assert((std::is_same<std::string,T>::value) == true);
        Array<std::string> V = copy();
        V.sort();

        std::stringstream stream;
        for(int i = 0; i < V.size(); i ++)
        {
            stream << V[i];
            if(i != V.size()-1)
                stream << delim;
        }
        return stream.str();
    }

    std::string strjoin_uniq(const std::string& delim=",")
    {
        assert((std::is_same<std::string,T>::value) == true);
        Array<std::string> V = copy();
        V.uniq();
        V.sort();

        std::stringstream stream;
        for(int i = 0; i < V.size(); i ++)
        {
            stream << V[i];
            if(i != V.size()-1)
                stream << delim;
        }
        return stream.str();
    }

    Array<T> intersection(Array<T>& B)
    {
        check_1d();
        std::unordered_set<T> B_set(B.begin(), B.end());

        std::vector<T> C;
        for(Iterator it = begin(); it != end(); it ++)
            if(B_set.find(*it) != B_set.end())
                C.push_back(*it);

        Array<T> D( { long( C.size() ) } );
        for(long i = 0; i < C.size(); i ++)
            D(i) = C[i];
        return D;
    }

    Array<T> diff(Array<T>& B)
    {
        check_1d();
        std::unordered_set<T> B_set(B.begin(), B.end());
        std::vector<T> C;
        for(Iterator it = begin(); it != end(); it ++)
            if(B_set.find(*it) == B_set.end())
                C.push_back(*it);

        Array<T> D( { long( C.size() ) } );
        for(long i = 0; i < C.size(); i ++)
            D(i) = C[i];
        return D;
    }

    template <class Lambda> Array<T> select(const Lambda& f)
    {
        check_1d();
        std::vector<T> v;
        for(Iterator it = begin(); it != end(); it ++)
            if( f( *it ) )
                v.push_back( *it );

        Array<T> A({ v.size() });
        for(long i = 0; i < v.size(); i ++)
            A(i) = v[i];

        return A;
    }

    Array<T> select() { return select( [](T x) -> bool { return x; } ); }

    void map( T (*FP)(T) )
    {
        if(!size())
            return;

        Iterator it = this->begin();
        if( it.is_contiguous() )
        {
            T* p = &(*it);
            long N = this->size();
            for(long i = 0; i < N; i ++)
            {
                *p = FP(*p);
                p ++;
            }
        }
        else
        {
            ForEach foreach(*this);
            for(T* p = foreach.begin(); p != nullptr; p = foreach.next())
                *p = FP(*p);
        }
    }

    void map(Array<T>& A, T (*FP)(T))
    {
        assert(A.shape() == this->shape());
        Iterator i = A.begin();
        Iterator end = A.end();
        Iterator j = this->begin();

        for(; i != end; ++ i)
        {
            *j = FP( *i );
            j ++;
        }
    }

    void map(Array<T>& A, T (*FP)(T,T))
    {
        assert(A.shape() == this->shape());

        /*if(this->is_phys() && A.is_phys())
        {
            this->print_config();
            A.print_config();

            //mars_log( this->m_phys_strides );
            //mars_log( this->m_log_slices.to_string() );

            //mars_log( std::distance( begin(), end() ) );
            //mars_log( size() );

            //T* p2 = begin().operator->();

            //mars_log( &( *begin() ) );

            //mars_log(p2);
            //exit(1);

            T* p = A.address();
            T* q = this->address();
            for(long i = 0; i < size(); ++ i)
                q[i] = FP( p[i], q[i] );
        }
        else
        */
        {
            Iterator i = A.begin();
            Iterator end = A.end();
            Iterator j = this->begin();

            for(; i != end; ++ i)
            {
                *j = FP( *i, *j );
                j ++;
            }
        }
    }

    T max() { return *(std::max_element( begin(), end() )); }
    T min() { return *(std::min_element( begin(), end() )); }
    long max_index() { return std::distance( begin(), std::max_element( begin(), end() ) ); }
    long min_index() { return std::distance( begin(), std::min_element( begin(), end() ) ); }
    T sum()
    {
        //if(size() < 500)
        {
            Iterator begin = this->begin();
            Iterator end   = this->end();

            //fprintf(stderr, "begin=%p\n", begin.operator->());
            //fprintf(stderr, "end  =%p\n", end.operator->());
            //mars_log( std::distance(begin.operator->(), end.operator->()) );

            //mars_log( begin.is_contiguous() );
            //assert(begin.is_contiguous());
            //exit(1);

            return std::accumulate( begin, end, T() );
        }
        /*
        else
            return sum_by_thread();
        */
    }
    T avg() { return size() ? sum() / double( size() ) : T(); }
    T stdev()
    {
        if(!size())
            return T(0);
        T avg = this->avg();
        T total = sum( [avg](T& x) { return (x-avg)*(x-avg); } );
        return sqrt( total / double( size() ) );
    }

    T sum_by_thread()
    {
        const int t_NUM = 4;
        const int N = size();

        pthread_t tids[ t_NUM ];
        ThreadData<T> ins[ t_NUM ];
        for(long t = 0; t < t_NUM; t ++)
        {
            ins[t].array = this;
            ins[t].first = N / t_NUM * t;
            ins[t].last  = N / t_NUM * (t+1);
            ins[t].result = T();
        }

        //mars_log(size());

        for(long t = 0; t < t_NUM; ++ t)
            pthread_create( &(tids[t]), NULL, mars::array_sum<T>, &( ins[t] ) );
        for(long t = 0; t < t_NUM; ++ t)
            pthread_join( tids[t], NULL );

        T sum = 0;
        for(long t = 0; t < t_NUM; ++ t)
            sum += ins[t].result;
        return sum;
    }

    template <class Lambda> T avg(const Lambda& f) { return size() ? sum(f) / double( size() ) : T(); }
    template <class Lambda> T sum(const Lambda& f)
    {
        if(!size())
            return T(0);

        T s = 0;
        Iterator end = this->end();
        for(Iterator it = begin(); it != end; it ++)
            s += f(*it);
        return s;
    }

    T product() { return size() ? std::accumulate(begin(), end(), T(1), std::multiplies<T>()) : T(); }

    template<class OP> T reduce(OP op, T rv) const
    { for(Iterator it = begin(); it != end(); it ++) rv = op(rv, *it); return rv; }

    T tss() { return sum( [](T& x) { return x*x; } ); }
    void floor() { map([](T x) { return std::max(x, T(0)); }); }
    void ceil() { map([](T x) { return std::min(x, T(0)); }); }

    Array group(const StackVector<long>& axes)
    {
        StackVector<long> shape = this->shape();

        StackVector<long> G_shape( axes.size() );
        for(long k = 0; k < axes.size(); k ++)
            G_shape[k] = shape[ axes[k] ];
        Array G( G_shape );
        G.range(0,0);

        StackVector<long> indices  (   shape.size() );
        StackVector<long> G_indices( G_shape.size() );

        switch(rank())
        {
        case 1:
            for(long i0 = 0; i0 < shape[0]; ++ i0)
            {
                indices[0] = i0;
                for(long k = 0; k < axes.size(); ++ k)
                    G_indices[k] = indices[ axes[k] ];
                G( G_indices ) += this->operator()(i0);
            }
            break;
        case 2:
            for(long i0 = 0; i0 < shape[0]; ++ i0)
            for(long i1 = 0; i1 < shape[1]; ++ i1)
            {
                indices[0] = i0;
                indices[1] = i1;
                for(long k = 0; k < axes.size(); ++ k)
                    G_indices[k] = indices[ axes[k] ];
                G( G_indices ) += this->operator()(i0, i1);
            }
            break;
        case 3:
            for(long i0 = 0; i0 < shape[0]; ++ i0)
            for(long i1 = 0; i1 < shape[1]; ++ i1)
            for(long i2 = 0; i2 < shape[2]; ++ i2)
            {
                indices[0] = i0;
                indices[1] = i1;
                indices[2] = i2;
                for(long k = 0; k < axes.size(); ++ k)
                    G_indices[k] = indices[ axes[k] ];
                G( G_indices ) += this->operator()(i0, i1, i2);
            }
            break;
        case 4:
            for(long i0 = 0; i0 < shape[0]; ++ i0)
            for(long i1 = 0; i1 < shape[1]; ++ i1)
            for(long i2 = 0; i2 < shape[2]; ++ i2)
            for(long i3 = 0; i3 < shape[3]; ++ i3)
            {
                indices[0] = i0;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i3;
                for(long k = 0; k < axes.size(); ++ k)
                    G_indices[k] = indices[ axes[k] ];
                G( G_indices ) += this->operator()(i0, i1, i2, i3);
            }
            break;
        case 5:
            for(long i0 = 0; i0 < shape[0]; ++ i0)
            for(long i1 = 0; i1 < shape[1]; ++ i1)
            for(long i2 = 0; i2 < shape[2]; ++ i2)
            for(long i3 = 0; i3 < shape[3]; ++ i3)
            for(long i4 = 0; i4 < shape[4]; ++ i4)
            {
                indices[0] = i0;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i3;
                indices[4] = i4;
                for(long k = 0; k < axes.size(); ++ k)
                    G_indices[k] = indices[ axes[k] ];
                G( G_indices ) += this->operator()(i0, i1, i2, i3, i4);
            }
            break;
        default:
            mars_log( rank() );
            assert( rank() > 0 );
            assert( rank() < 6 );
        }

        return G;
    }

    void normalize_sum_1()
    {
        T total = sum();
        //mars_log(total);
        assert( total != 0 );
        //const long M = size();
        //print();
        for(Iterator it = begin(); it != end(); it ++)
            *it /= total;
        //print();
    }

    void check_shape(const Array<T>& rhs) const
    {
        ASSERT_EQ( shape(), rhs.shape() );
    }
/*
#define compound_assignment_impl2(OP) \
    Array<T>& operator OP(Array<T>& rhs) \
    { \
        check_shape(rhs); \
        ForEach f(*this), g(rhs);  \
        T* p=f.begin(); \
        T* q=g.begin(); \
        for(; p!=nullptr; p=f.next(), q=g.next()) \
            *p OP *q; \
        return *this; \
    }
*/
#define compound_assignment_impl(OP) \
Array<T>& operator OP(const Array<T>& rhs) \
{ \
    check_shape(rhs); \
    if(this->is_phys() && rhs.is_phys()) \
    { \
        T* p = this->address(); \
        T* q = rhs.address(); \
        for(long i = 0; i < size(); ++ i) \
            p[i] OP q[i]; \
    } \
    else \
    { \
        Iterator i = this->begin(); \
        CIterator j = rhs.cbegin(); \
        for(; i != this->end(); ++ i) \
        { \
            *i OP *j; \
            j ++; \
        } \
    } \
    return *this; \
}\
\
Array<T>& operator OP(Array<T>& rhs) \
{ \
    check_shape(rhs); \
    Iterator i = this->begin(); \
    CIterator j = rhs.cbegin(); \
    Iterator end = this->end(); \
    for(; i != end; ++ i) \
    { \
        *i OP *j; \
        j ++; \
    } \
    return *this; \
}

compound_assignment_impl(+=);
compound_assignment_impl(-=);
compound_assignment_impl(*=);
compound_assignment_impl(/=);
compound_assignment_impl(%=);

#define compound_assignment_by_element_impl(OP) \
Array<T>& operator OP(const T& rhs) \
{ \
    if(!size()) \
        return *this; \
    Iterator it = this->begin(); \
    if( it.is_contiguous() ) \
    { \
        T* p = &(*it); \
        long N = size(); \
        for(long i = 0; i < N; i ++) \
        { \
            *p OP rhs; \
            p ++; \
        } \
    } \
    else \
    { \
        ForEach f(*this); \
        for(T* p = f.begin(); p != nullptr; p = f.next()) \
            *p OP rhs; \
    } \
    return *this; \
}

compound_assignment_by_element_impl(=);
compound_assignment_by_element_impl(+=);
compound_assignment_by_element_impl(-=);
compound_assignment_by_element_impl(*=);
compound_assignment_by_element_impl(/=);
compound_assignment_by_element_impl(%=);

    void log_constructor()
    {
#ifdef DEBUG_MARS_ARRAY
        fprintf(stderr, "Create Array<%s>(size=%ld, owner=%d, phys=%d): %p\n",  typeid(T).name(), size(), m_own, m_phys, m_p);
#endif
    }

    void log_destructor()
    {
#ifdef DEBUG_MARS_ARRAY
        if(owner()) fprintf(stderr, "Destroy Array<%s>(size=%ld, owner=%d, phys=%d): %p\n",  typeid(T).name(), size(), m_own, m_phys, m_p);
#endif
    }

    void fft_fft_inv_impl(Array<std::complex<double> >& B, int sign)
    {
        Array<std::complex<double> >& A = *this;


        fftw_complex* pA = reinterpret_cast<fftw_complex*>( A.address() );
        fftw_complex* pB = reinterpret_cast<fftw_complex*>( B.address() );

        fftw_plan plan;
        unsigned int flags = FFTW_ESTIMATE;
        StackVector<long> N = A.shape();
        switch(A.rank())
        {
            case 1: plan = fftw_plan_dft_1d(N[0],             pA, pB, sign, flags); break;
            case 2: plan = fftw_plan_dft_2d(N[0], N[1],       pA, pB, sign, flags); break;
            case 3: plan = fftw_plan_dft_3d(N[0], N[1], N[2], pA, pB, sign, flags); break;
            default: assert( A.rank() < 4 );
        }

        fftw_execute(plan);
        fftw_destroy_plan(plan);
    }

    Array<std::complex<double> > fft()
    {
        Array<std::complex<double> > B(*this);
        fft_fft_inv_impl(B, FFTW_FORWARD);
        return B;
    }

    Array<std::complex<double> > fft_inv()
    {
        Array<std::complex<double> > B(*this);
        fft_fft_inv_impl(B, FFTW_BACKWARD);
        return B;
    }



};

#define arithmetic_op_by_element_impl(OP, OP2) \
template <class T> Array<T> \
operator OP(Array<T>& lhs, const T& rhs) \
{ \
    Array<T> new_lhs = lhs.copy(); \
    new_lhs OP2 rhs; \
    return new_lhs; \
}

arithmetic_op_by_element_impl(+, +=);
arithmetic_op_by_element_impl(-, -=);
arithmetic_op_by_element_impl(*, *=);
arithmetic_op_by_element_impl(/, /=);

#define arithmetic_op_impl(OP, OP2) \
template <class T> Array<T> \
operator OP(Array<T>& lhs, Array<T>& rhs) \
{ \
    Array<T> new_lhs = lhs.copy(); \
    new_lhs OP2 rhs; \
    return new_lhs; \
}

arithmetic_op_impl(+, +=);
arithmetic_op_impl(-, -=);
arithmetic_op_impl(*, *=);
arithmetic_op_impl(/, /=);

/*
template <class T=double>
Array<T> to_array(const std::vector<std::vector<T> >& V) // this has to be different from the constructor
{
    assert(V.size() > 0);
    assert(V[0].size() > 0);

    Array<T> A( { V.size(), V[0].size() } );
    for(long i = 0; i < V.size(); i ++)
    {
        assert( V[0].size() == V[i].size() );
        for(long j = 0; j < V[i].size(); j ++)
            A(i, j) = V[i][j];
    }
    return A;
}
*/


#define to_array_1d_impl(T) \
Array<T> to_array(std::vector<T>& V) \
{ \
    assert(V.size() > 0); \
    Array<T> A( StackVector<long>{ long( V.size() ) } ); \
    for(long i = 0; i < V.size(); i ++) \
        A(i) = V[i]; \
    return A; \
}\
\
Array<T> to_array(const std::vector<T>& V) \
{ \
    assert(V.size() > 0); \
    Array<T> A( StackVector<long>{ long( V.size() ) } ); \
    for(long i = 0; i < V.size(); i ++) \
        A(i) = V[i]; \
    return A; \
}

#define to_array_2d_impl(T) \
Array<T> to_array(std::vector<std::vector<T> >& V) \
{ \
    assert(V.size() > 0); \
    Array<T> A( StackVector<long>{ long( V.size() ), long( V[0].size() ) } ); \
    for(long i = 0; i < V.size(); i ++) \
        for(long j = 0; j < V[0].size(); j ++) \
            A(i,j) = V[i][j]; \
    return A; \
} \
\
Array<T> to_array(const std::vector<std::vector<T> >& V) \
{ \
    assert(V.size() > 0); \
    Array<T> A( StackVector<long>{ long( V.size() ), long( V[0].size() ) } ); \
    for(long i = 0; i < V.size(); i ++) \
        for(long j = 0; j < V[0].size(); j ++) \
            A(i,j) = V[i][j]; \
    return A; \
}

to_array_1d_impl(float);
to_array_1d_impl(double);
to_array_1d_impl(std::string);
to_array_1d_impl(long);
to_array_1d_impl(int);

to_array_2d_impl(float);
to_array_2d_impl(double);
to_array_2d_impl(std::string);
to_array_2d_impl(long);
to_array_2d_impl(int);

}

typedef mars::Array<std::string > SArray_t;
typedef mars::Array<long> IArray_t;
typedef mars::Array<std::complex<double> > CArray_t;
typedef mars::Array<double> Array_t;

#endif
