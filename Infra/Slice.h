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

#ifndef MARS_TYPE_SLICE_H
#define MARS_TYPE_SLICE_H 1

namespace mars{

class Slice
{
private:
    long m_begin;
    long m_end;
    long m_step;
    long m_size;

public:

    inline Slice() : m_begin(0), m_end(LONG_MAX), m_step(1)
    {
        //mars_log_code();
        //iter = 1;
    }

    void init_by_vector(std::vector<long> ns)
    {
        if(ns.size() == 0 )
        {
            m_begin = 0;
            m_end = LONG_MAX;
            m_step = 1;
            //mars_log_code();
        }
        else if(ns.size() == 1 )
        {
            m_begin = ns[0];
            m_end = LONG_MAX;
            m_step = 1;
        }
        else if(ns.size() == 2 )
        {
            m_begin = ns[0];
            m_end = ns[1];
            m_step = 1;
        }
        else if(ns.size() == 3 )
        {
            m_begin = ns[0];
            m_end = ns[1];
            m_step = ns[2];
        }
        else
            assert(ns.size() <= 3);
        //iter = m_begin;

        set_size();
    }

    //template <class T>
    Slice(std::initializer_list<long> l)
    {
        //mars_log_code();
        /*
        std::vector<long> ns;
        for(long i = 0; i < l.size(); i ++)
            ns.push_back(l[i]);
        */

        std::vector<long> ns( l );
        init_by_vector(ns);
        //std::cout << "constructed with a " << l.size() << "-element list\n";
    }

    inline Slice(std::vector<long> ns)
    {
        init_by_vector(ns);
    }

    inline Slice(long a, long b, long c) : m_begin(a), m_end(b), m_step(c)
    {
        //mars_log_code();
        //iter = m_begin;
        assert(m_step != 0);
    }

    inline Slice(long a) : m_begin(a), m_end(a+1), m_step(1)
    {
        //mars_log_variable(a);
        //mars_log_code();
        //iter = m_begin;
        assert(m_step != 0);
    }

    inline Slice(const mars::Slice& S_i)
    {
        //mars_log_code();
        this->m_begin = S_i.m_begin;
        this->m_end = S_i.m_end;
        this->m_step = S_i.m_step;
        this->m_size = S_i.m_size;
    }

    // Iterator functions
    //inline bool operator!= (const Slice&) const { return iter < m_end; }
    //inline void operator++ () { iter += m_step; }
    //inline long operator* () const { return iter; }

    inline long begin() const { return m_begin; }
    inline long end() const { return m_end; }
    inline long step() const { return m_step; }

    inline void set_begin (long a) { m_begin = a; set_size(); }
    inline void set_end (long b) { m_end = b; set_size(); }
    inline void set_step (long c) { m_step = c; set_size(); }

    inline long order (const long& i) const { return (i-m_begin) / m_step; }

    inline void set_size() { m_size = long( std::ceil( double(m_end - m_begin) / double(m_step) ) ); }

    inline long size() const
    {
        //fprintf(stderr, "m_begin:m_end:m_step=%ld:%ld:%ld\n", m_begin, m_end, m_step);
        return m_size;
    }

    std::string to_string() const
    {
        std::stringstream stream;
        for(long i = m_begin; i < m_end; i += m_step)
            stream<< i << " ";
        return stream.str();
    }

    void print() const
    {
        for(long i = m_begin; i < m_end; i += m_step)
            fprintf(stderr, "%ld ", i);
        fprintf(stderr, "\n");
    }
};

}

#define SLICE_WHOLE mars::Slice(0, LONG_MAX, 1)

#endif

