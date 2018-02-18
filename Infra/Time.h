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
#ifndef MARS_TIME_H
#define MARS_TIME_H 1

namespace mars{

class Time
{
public:
    void validate(const struct tm& date)
    {
        try
        {
            assert(date.tm_sec >= 0 && date.tm_sec <= 60);
            assert(date.tm_min >= 0 && date.tm_sec <= 59);
            assert(date.tm_hour >= 0 && date.tm_hour <= 23);

            assert(date.tm_mday >= 1 && date.tm_mday <= 31);
            assert(date.tm_mon >= 0 && date.tm_mon <= 11);
            assert(date.tm_year >= 0);
        }
        catch(std::exception& e)
        {
            std::cerr<< e.what() << std::endl;
        }
    }

    void set_time(int year, int month, int day, int hour, int minute, int second)
    {
        struct tm date;
        date.tm_sec = second;
        date.tm_min = minute;
        date.tm_hour = hour;

        date.tm_year = year - 1900;
        date.tm_mon = month - 1;  // note: zero indexed
        date.tm_mday = day;       // note: not zero indexed
        validate(date);
        date.tm_isdst = false;
        m_time = mktime( &date );
    }

    Time()
    {}

    Time(std::string s)
    {
        mars::String S;
        std::vector<std::string> v = S.strsplits(s, "/", false);
        assert(v.size() == 3);
        int month = std::atoi(v[0].c_str());
        int day = std::atoi(v[1].c_str());
        int year = std::atoi(v[2].c_str());
        //fprintf(stderr, "%d/%d/%d\n", month, day, year);
        set_time(year, month, day, 12, 0, 0);
    }

    Time(int year, int month, int day)
    {
        set_time(year, month, day, 12, 0, 0);
    }

    Time(int year, int month, int day, int hour, int minute, int second)
    {
        set_time(year, month, day, hour, minute, second);
    }

    Time(time_t t)
    {
        m_time = t;
    }

    time_t get_data()
    {
        return( m_time );
    }

    Time operator +(int days) const
    {
        const time_t ONE_DAY = 24 * 60 * 60;
        return(Time( m_time + (days * ONE_DAY) ));
    }

    Time operator -(int days) const
    {
        return(operator+((-1) * days));
    }

    ~Time()
    {}

    std::string to_string() const
    {
        struct tm date;
        date = *localtime( &m_time );
        std::stringstream out;
        out<< asctime( &date );
        std::string s = out.str();
        return s.substr(0, s.size() - 1);
    }

    void print() const
    {
        std::cerr<< to_string() << std::endl;
    }

    friend double operator -(Time t0, Time t1) { return difftime(t0.m_time, t1.m_time); }

#define logical_op(OP) friend bool operator OP(Time t0, Time t1) { return t0.m_time OP t1.m_time; }
    logical_op(>)
    logical_op(>=)
    logical_op(==)
    logical_op(<=)
    logical_op(<)
    logical_op(!=)
#undef logical_op

protected:
    time_t m_time;
};

}

#endif
