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
#ifndef MARS_DATE_H
#define MARS_DATE_H 1

namespace mars{

class Date : public mars::Time
{
public:
    Date()
    {}

    Date(Time s) : Time(s)
    {}

    Date(std::string s) : Time(s)
    {}

    Date(int year, int month, int day) : Time(year, month, day)
    {}

    time_t get_data()
    {
        return( mars::Time::get_data() );
    }

    Date operator +(int days) const
    {
        //mars_log_variable(days);
        mars::Time t = mars::Time::operator+(days);
        //t.print();
        return( t );
    }

    Date operator -(int days) const
    {
        //mars_log_variable(days);
        mars::Time t = mars::Time::operator-(days);
        //t.print();
        return( t );
    }

    std::string to_string() const
    {
        struct tm date;
        date = *localtime( &(mars::Time::m_time) );

        char result[11];
        sprintf(result, "%.2d/%.2d/%.4d",
            date.tm_mon + 1,
            date.tm_mday,
            date.tm_year + 1900);
            return result;
    }

    void print() const
    {
        std::cerr<< to_string() << std::endl;
    }



    ~Date()
    {}
};

}

#endif
