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
#ifndef MARS_TIMECURVE_H
#define MARS_TIMECURVE_H 1

namespace mars{

class TimeCurve
{
public:
    TimeCurve()
    {}

    long size() const
    {
        return m_data.size();
    }

    std::vector<mars::Time> keys() const
    {
        std::vector<mars::Time> ks( size() );

        long i = 0;
        for(auto it = m_data.begin(); it != m_data.end(); it ++)
        {
            ks[i] = it->first;
            i ++;
        }
        std::sort(ks.begin(), ks.end());
        return ks;
    }

    std::vector<double> values() const
    {
        std::vector<mars::Time> ks = keys();
        std::vector<double> vs( size() );
        for(long i = 0; i < ks.size(); i ++)
        {
            vs[i] = m_data.at( ks[i] );
        }

        return vs;
    }

    double& operator [](const mars::Date& k)
    {
        return( m_data[k] );
    }

    const double& operator [](const mars::Date& k) const
    {
        return( m_data.at(k) );
    }

    double& at (const mars::Time& k)
    {
        return( m_data[k] );
    }

    const double& at (const mars::Time& k) const
    {
        return( m_data.at(k) );
    }

    void print()
    {
        std::vector<mars::Time> ks = keys();
        for(const auto& k : ks)
            fprintf(stderr, "%s:%f\n", k.to_string().c_str(), m_data[k]);
    }

private:
    std::map<mars::Time, double> m_data;
};

}

#endif
