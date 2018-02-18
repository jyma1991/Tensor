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

#ifndef MARS_TYPE_STACKVECTOR_H
#define MARS_TYPE_STACKVECTOR_H 1

#define DEBUG_TYPE_STACKVECTOR_H 1
#undef  DEBUG_TYPE_STACKVECTOR_H

namespace mars{

template <typename T>
class StackVector
{
private:
    enum { MAX_SIZE = 6 };
    T m_p[MAX_SIZE];
    int m_size;

    void check_size()
    {
#ifdef DEBUG_TYPE_STACKVECTOR
        assert( size() < MAX_SIZE );
#endif
    }

public:
    explicit StackVector() : m_size(0) {}
    explicit StackVector(long size_i) : m_size(int(size_i)) { check_size(); }
    explicit StackVector(long size_i, T x) : m_size(int(size_i))
    {
        check_size();
        for(int i = 0; i < m_size; i ++)
            m_p[i] = x;
    }

    template <typename Base>
    explicit StackVector(const Base* begin_i, const Base* end_i) : m_size(0)
    {
        for(const Base* p = begin_i; p != end_i; p ++)
        {
            m_size ++;
            check_size();
            m_p[m_size-1] = T(*p);
        }
    }

    StackVector(const std::initializer_list<T>& v_i)
    {
        m_size = v_i.size();
        int i = 0;
        for(auto& x : v_i)
        {
            m_p[i] = x;
            i ++;
        }
    }

    StackVector(const StackVector<T>& rhs) : m_size(rhs.size())
    {
        check_size();
        std::memcpy(m_p, &(rhs[0]), sizeof(T) * m_size);
    }

    StackVector& operator=( const StackVector<T>& rhs )
    {
        m_size = rhs.size();
        std::memcpy(m_p, &(rhs[0]), sizeof(T) * m_size);
        return *this;
    }

    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    iterator begin() noexcept { return iterator(m_p); }
    const_iterator begin() const noexcept { return const_iterator(m_p); }
    iterator end() noexcept { return iterator(m_p + m_size); }
    const_iterator end() const noexcept { return const_iterator(m_p + m_size); }
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_iterator cbegin() const noexcept { return const_iterator(m_p); }
    const_iterator cend() const noexcept { return const_iterator(m_p + m_size); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }
    constexpr size_type max_size() const noexcept { return MAX_SIZE; }

    inline bool empty() const noexcept { return size() == 0; }

    inline int size() const { return m_size; }
    inline void resize(long i) { check_size(); m_size = i; }

    inline T& operator[](long i) { return m_p[i]; }
    inline const T& operator[](long i) const { return m_p[i]; }

    //T operator[](long i) { return m_p[i]; }

    inline void push_back(T value_i)
    {
        m_size ++;
        check_size();
        m_p[m_size-1] = value_i;
    }

    StackVector<T> join(const StackVector<T>& rhs)
    {
        int M = this->size();
        int N = rhs.size();
        StackVector<T> v(M + N);
        for(int i = 0; i < M; ++ i)
            v[i] = (*this)[i];
        for(int i = M; i < M+N; ++ i)
            v[i] = rhs[i-M];

        return v;
    }

    std::string to_string() const
    {
        std::stringstream stream;
        stream << "(";
        int N = size();
        for(int i = 0; i < N; ++ i)
        {
            stream << m_p[i];
            if(i != N-1)
                stream << ",";
        }
        stream << ")";
        return stream.str();
    }

    void print() const { fprintf(stderr, "%s\n", to_string().c_str()); }

    friend std::ostream& operator<< (std::ostream& stream, const StackVector& v) { return stream << v.to_string(); }

    T product()
    {
        T x;
        if( size() > 0 )
        {
            x = 1;
            for(long i = 0; i < size(); i ++)
                x *= operator[](i);
        }
        else
            x = 0;

        return  x;
    }
};

template <typename T>
inline bool operator==(const StackVector<T>& lhs, const StackVector<T>& rhs) { return std::equal(lhs.begin(), lhs.end(), rhs.begin()); }

template <typename T>
inline bool operator!=(const StackVector<T>& lhs, const StackVector<T>& rhs) { return !( operator==(lhs, rhs) ); }

typedef mars::StackVector<long> Vector_t;

}

#endif
