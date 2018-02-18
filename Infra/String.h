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

#ifndef MARS_TYPE_STRING_H
#define MARS_TYPE_STRING_H 1

namespace mars{

class PrintOptions
{
private:
    std::map<std::string, std::string> m_opts;

public:
    template <class T> void set_opt(std::string F) { m_opts[typeid(T).name()] = F; }
    template <class T> std::string get_opt() { return m_opts[typeid(T).name()]; }
    std::map<std::string, std::string> get_options() { return m_opts; }

    PrintOptions()
    {
        set_opt<char>("%c");
        set_opt<signed char>("%c");
        set_opt<unsigned char>("%c");

        set_opt<short>("%d");
        set_opt<unsigned short>("%u");

        set_opt<int>("%d");
        set_opt<unsigned int>("%u");

        set_opt<long>("%ld");
        set_opt<unsigned long>("%lu");

        set_opt<long long>("%lld");
        set_opt<unsigned long long>("%llu");

        set_opt<float>("%.2f");
        set_opt<double>("%.2f");

        set_opt<long double>("%lf");
        set_opt<std::complex<double> >("(%.2f,%.2f)");

        set_opt<std::string>("%s");
    }

    void print()
    {
        for(const auto& v : m_opts)
            fprintf(stderr, "%s:%s\n", v.first.c_str(), v.second.c_str());
    }

    ~PrintOptions() {}
};

static PrintOptions opts;

class String
{
private:
std::string m_delim;
std::vector<std::string> m_bracket;

char lower_char(const char& c) { return (c >= 'A' && c <= 'Z') ? char(c - ('Z'-'z')) : c; }
char upper_char(const char& c) { return (c >= 'a' && c <= 'z') ? char(c + ('Z'-'z')) : c; }

public:
    String() : m_delim( "," ), m_bracket{ "(", ")" } {}

/*
if output is returned, this will be a bug. '\0' will be kept in the string and
string stream.str() does not work any more as it truncates at '\0'.
*/
std::string sprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    uint64_t size = std::vsnprintf( NULL, 0, format, args );

    va_start(args, format);
    std::string output( size + 1, '_' );
    std::vsprintf( &output[0], format, args );
    va_end(args);

    std::string filtered(output.c_str());
    return filtered;
}



void set_delim(std::string X) { m_delim = X; }
void set_bracket(std::string X, std::string Y)
{
    m_bracket.resize(2);
    m_bracket[0] = X;
    m_bracket[1] = Y;
}

std::string strlower(std::string str) { for(std::string::iterator it = str.begin(); it != str.end(); it ++) *it = lower_char(*it); return str; }
std::string strupper(std::string str) { for(std::string::iterator it = str.begin(); it != str.end(); it ++) *it = upper_char(*it); return str; }
bool strbegins(std::string str, std::string delim) { assert( delim.size() ); return( str.find(delim, 0) == 0 ); }
bool strcontains(std::string str, std::string delim) { assert( delim.size() ); return( str.find(delim, 0) != std::string::npos ); }
std::string strreverse(std::string str) { std::string s = str; for(int i = 0; i < (int)s.size(); i ++) s[i] = str[s.size()-1 - i]; return s; }

int strends(std::string s_input, std::string suffix_input)
{
    const char* str = s_input.c_str();
    const char* suffix = suffix_input.c_str();

    if (!str || !suffix)
        return 0;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if(lensuffix > lenstr)
        return 0;

    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

std::vector<std::string> strsplits_impl(const std::string &str, std::string delim)
{
    std::vector<std::string> array;

    size_t pos = 0;
    size_t found;

    for(;;)
    {
        found = str.find(delim, pos);

        std::string substr = str.substr(pos, found-pos);
        if( substr.size() )
            array.push_back( substr );

        if(found != std::string::npos)
            pos = found + delim.size();
        else
            break;
    }

    return array;
}

std::vector<std::string> strsplits(const std::string &s, std::string delim, int use_quote)
{
    std::vector<std::string> array;
    int quotes = 0;
    long start = 0;
    for(int i = 0; i < s.size(); i ++)
    {
        int pos = -1;
        if(s[i] == '\"')
            quotes ++;
        if(s.substr(i, delim.size()) == delim)
        {
            if(use_quote == true)
            {
                if(quotes % 2 == 0)
                    pos = i;
            }
            else
                pos = i;
        }
        if(pos != -1)
        {
            array.push_back( s.substr(start, pos-start) );
            start = pos+delim.size();
        }
    }

    array.push_back( s.substr(start) );

    return array;
}

std::vector<std::string> strsplits(const std::string &str_input, std::string delim_0, std::string delim_1)
{
    std::vector<std::string> strs = strsplits_impl(str_input, delim_0);
    std::vector<std::string> array;

    for(int i = 0; i < strs.size(); i ++)
    {
        std::vector<std::string> v = strsplits_impl(strs[i], delim_1);
        for(int j = 0; j < v.size(); j ++)
            array.push_back( v[j] );
    }

    return array;
}

std::string strjoin(const std::vector<std::string>& vs, const std::string& delim)
{
    std::stringstream stream;
    for(int i = 0; i < vs.size(); i ++)
    {
        stream << vs[i];
        if(i != vs.size()-1)
            stream << delim;
    }
    return stream.str();
}

std::string strreplace(std::string str_input, std::string x, std::string y)
{
    size_t pos = 0;
    size_t found;
    std::string str = str_input;

    for(;;)
    {
        found = str.find(x, pos);
        if(found != std::string::npos)
            str = str.substr(0, found) + y + str.substr(found + x.size() );
        else
            break;
    }

    return str;
}

private:
    std::string cons(const int& x) { return sprintf( opts.get_opt<int>().c_str(), x ); }
    std::string cons(const long& x) { return sprintf( opts.get_opt<long>().c_str(), x ); }
    std::string cons(const double& x) { return sprintf( opts.get_opt<double>().c_str(), x ); }
    std::string cons(const std::string& x) { return x; }

    template <typename T>
    std::string cons(const T& x) { return sprintf( "%p", x ); }

    std::string cons(const bool& x) { return x ? "true" : "false"; }
    std::string cons(const std::complex<double>& x) { return sprintf( opts.get_opt<std::complex<double> >().c_str(), std::real(x), std::imag(x) ); }

    template<class T>
    std::string cons(const std::vector<T>& V)
    {
        std::vector<std::string> U(V.size());
        for(long i = 0; i < U.size(); i ++)
            U[i] = cons(V[i]);
        return m_bracket[0] + strjoin(U, m_delim) + m_bracket[1];
    }

    template<class T>
    std::string cons(const StackVector<T>& SV)
    {
        std::vector<T> v( &(SV[0]), &(SV[0]) + SV.size() );
        return mars::String().to_string( v );
    }

public:
std::map<std::string, std::string> get_print_option() { return opts.get_options(); }
template<class T> void set_printoptions(std::string value) { opts.set_opt<T>( value ); }

template <class T0>
std::string to_string(const T0& X0)
{ return cons(X0); }

template <class T0>
std::string to_string(const T0*& X0)
{ return cons<T0*>(X0); }

template <class T0,class T1>
std::string to_string(const T0& X0,const T1& X1)
{ return cons(X0)+m_delim+cons(X1); }

template <class T0,class T1,class T2>
std::string to_string(const T0& X0,const T1& X1,const T2& X2)
{ return cons(X0)+m_delim+cons(X1)+m_delim+cons(X2); }

template <class T0,class T1,class T2,class T3>
std::string to_string(const T0& X0,const T1& X1,const T2& X2,const T3& X3)
{ return cons(X0)+m_delim+cons(X1)+m_delim+cons(X2)+m_delim+cons(X3); }

template <class T0>
void print(const T0& X0)
{ fprintf(stderr, "%s\n", to_string(X0).c_str()); }

template <class T0,class T1>
void print(const T0& X0,const T1& X1)
{ fprintf(stderr, "%s\n", to_string(X0,X1).c_str()); }

template <class T0,class T1,class T2>
void print(const T0& X0,const T1& X1,const T2& X2)
{ fprintf(stderr, "%s\n", to_string(X0,X1,X2).c_str()); }

template <class T0,class T1,class T2,class T3>
void print(const T0& X0,const T1& X1,const T2& X2,const T3& X3)
{ fprintf(stderr, "%s\n", to_string(X0,X1,X2,X3).c_str()); }

void print_box_horizontal(long prefix, const std::vector<long>& widths)
{
    for(long k = 0; k < prefix + 1; k ++)
        fprintf(stderr, " " );
    fprintf(stderr, "+" );
    for(long j = 0; j < widths.size(); j ++)
        for(long k = 0; k < widths[j] + 1; k ++)
            fprintf(stderr, "-" );
    fprintf(stderr, "+\n");
}

void prettyprint_table_impl(const std::vector<std::vector<std::string> >& data_input, bool right_align, bool add_number)
{
    const long M = data_input.size();
    bool box = true;

    //first round
    long items = 0;
    for(long i = 0; i < M; i ++)
        items = std::max(items, long( data_input[i].size() ) );

    std::vector<long> widths( items, 0 );
    std::vector<std::string> formats( items );

    std::vector<std::vector<std::string> > data;
    data.push_back(std::vector<std::string>());
    if(box && add_number)
        for(long j = 0; j < items; j ++)
            data[0].push_back( std::to_string(j) );

    for(long i = 0; i < data_input.size(); i ++)
        data.push_back( data_input[i] );

    //second round
    for(long i = 0; i < data.size(); i ++)
        for(long j = 0; j < data[i].size(); j ++)
        {
            widths[j] = std::max(widths[j], long(data[i][j].size()));
        }

    for(long j = 0; j < widths.size(); j ++)
    {
        if(right_align)
            formats[j] = std::string("%") + std::to_string(widths[j]) + "s ";
        else
            formats[j] = std::string("%-") + std::to_string(widths[j]) + "s ";
    }

    //third round
    long prefix = std::ceil( log10( double(M) ) );
    if(box)
    {
        for(long k = 0; k < prefix + 2; k ++)
            fprintf(stderr, " ");

        for(long j = 0; j < data[0].size(); j ++)
            fprintf(stderr, formats[j].c_str(), data[0][j].c_str());
        if(add_number)
            fprintf(stderr, "\n");
        print_box_horizontal(prefix, widths);
    }

    for(long i = 1; i < M+1; i ++)
    {
        if(box && add_number)
        {
            std::string format = sprintf( "%%0%dd |", prefix );
            fprintf(stderr, format.c_str(), i-1 );
        }
        else
            fprintf(stderr, "  |");

        for(long j = 0; j < data[i].size(); j ++)
        {
            fprintf(stderr, formats[j].c_str(), data[i][j].c_str());
        }
        if(box)
            fprintf(stderr, "|\n");
        else
            fprintf(stderr, "\n");
    }
    if(box)
        print_box_horizontal(prefix, widths);
}

template <class T>
void prettyprint_table(const std::vector<std::vector<T> >& data_T, bool right_align=true, bool add_number=true)
{
    const long M = data_T.size();
    std::vector<std::vector<std::string> > data;//( data_T.size() , std::vector<std::string>() );

    for(long i = 0; i < M; i ++)
    {
        data.push_back( std::vector<std::string>() );
        for(long j = 0; j < data_T[i].size(); j ++)
        {
            //fprintf(stderr, "%f\n", data_T[i][j] );
            //fprintf(stderr, "%s\n", mars::infra::string::to_string( data_T[i][j] ).c_str() );
            data[i].push_back( to_string( data_T[i][j] ) );
        }
    }

    prettyprint_table_impl(data, right_align, add_number);
}

void prettyprint_table(const std::vector<std::vector<std::string> >& data, bool right_align=true, bool add_number=true)
{
    prettyprint_table_impl(data, right_align, add_number);
}

void prettyprint_table(const std::vector<std::pair<std::string, std::string > >& data, bool right_align=true, bool add_number=true)
{
    mars::String _;
    std::vector<std::vector<std::string > > mat;
    std::vector<std::string > v;

	for(long i = 0; i < data.size(); i ++)
	    v.push_back( data[i].first );
    mat.push_back( v );

    v.resize(0);
	for(long i = 0; i < data.size(); i ++)
	    v.push_back( data[i].second );
    mat.push_back( v );

    prettyprint_table_impl(mat, right_align, add_number);
}

/*
std::string to_string(const std::vector<double>& v)
{
    std::string delim = ",";
    std::vector<std::string> vs = mars::infra::vector::mapcar( [](double x) -> std::string
            {
                return( std::to_string( x ) );
            },
            v,
            std::string() );
    return mars::infra::string::join(vs, delim);
}

std::string to_string(const std::vector<long>& v)
{
    std::string delim = ",";
    std::vector<std::string> vs = mars::infra::vector::mapcar( [](long x) -> std::string
            {
                return( std::to_string( x ) );
            },
            v,
            std::string() );
    return mars::infra::string::join(vs, delim);
}

std::string to_string(const std::vector<int>& v)
{
    std::string delim = ",";
    std::vector<std::string> vs = mars::infra::vector::mapcar( [](int x) -> std::string
            {
                return( std::to_string( x ) );
            },
            v,
            std::string() );
    return mars::infra::string::join(vs, delim);
}

template<class T>
std::string to_string(const std::vector<T>& v, const std::string& delim)
{
    std::vector<std::string> vs = mars::infra::vector::mapcar( [](T x) -> std::string
            {
                return( std::to_string( x ) );
            },
            v,
            std::string() );
    return mars::infra::string::join(vs, delim);
}
*/

int levenshtein_min(int a, int b, int c)
{
    int min=a;
    if(b<min)
        min=b;
    if(c<min)
        min=c;
    return min;
}

/*Compute levenshtein distance between s and t*/
int levenshtein_distance(const std::string& sts, const std::string& stt)
{
    //Step 1
    int k,i,j,n,m,cost,*d,distance;
    n = int(sts.size());
    m = int(stt.size());
    if( n!=0 && m!=0 )
    {
        char* s = new char[ sts.size() + 1 ];
        strcpy( s, sts.c_str() );
        char* t = new char[ stt.size() + 1 ];
        strcpy( t, stt.c_str() );

        d = (int*) malloc((sizeof(int))*(m+1)*(n+1));
        m ++;
        n ++;
        //Step 2
        for(k=0; k<n; k++)
            d[k]=k;
        for(k=0; k<m; k++)
            d[k*n]=k;
        //Step 3 and 4
        for(i=1; i<n; i++)
            for(j=1; j<m; j++)
            {
                //Step 5
                if(s[i-1]==t[j-1])
                    cost=0;
                else
                    cost=1;
                //Step 6
                d[j*n+i]=levenshtein_min(d[(j-1)*n+i]+1,d[j*n+i-1]+1,d[(j-1)*n+i-1]+cost);
            }
        distance=d[n*m-1];
        free(d);

        delete [] s;
        delete [] t;

        return distance;
    }
    else
    {
        return -1; //a negative return value means that one or both strings are empty.
    }
}

/*
in  __levenshtein_distance(ch* s, ch* t)
{
    //Step 1
    in  k,i,j,n,m,cost,*d,distance;
    n=strlen(s);
    m=strlen(t);
    if(n!=0&&m!=0)
    {
        d= (in*) malloc((sizeof(in))*(m+1)*(n+1));
        m ++;
        n ++;
        //Step 2
        for(k=0;k<n;k++)
	        d[k]=k;
        for(k=0;k<m;k++)
            d[k*n]=k;
        //Step 3 and 4
        for(i=1;i<n;i++)
            for(j=1;j<m;j++)
	        {
                //Step 5
                if(s[i-1]==t[j-1])
                    cost=0;
                else
                    cost=1;
                //Step 6
                d[j*n+i]=levenshtein_min(d[(j-1)*n+i]+1,d[j*n+i-1]+1,d[(j-1)*n+i-1]+cost);
            }
        distance=d[n*m-1];
        free(d);
        return distance;
    }
    else
        return -1; //a negative return value means that one or both strings are empty.
}
*/

private:
template <int N>
std::string base_conversion(long v)
{
    bool flag = v >= 0;
    if(!flag)
        v = -v;

    const char* digits;
    switch(N)
    {
    case 2:
        digits = "01";
        break;
    case 8:
        digits = "01234567";
        break;
    case 10:
        digits = "0123456789";
        break;
    case 16:
        digits = "0123456789ABCDEF";
        break;
    case 36:
        digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        break;
    default:
        throw( std::runtime_error( std::to_string(N) + " not supported by base_conversion" ) );
    }

    long quot, rem;
    std::vector<long> rems;
    for(;;)
    {
        quot = v / N;
        rem  = v % N;
        rems.push_back(rem);
        if(quot <= 0)
            break;
        v = quot;
    }
    std::reverse( rems.begin(), rems.end() );

    const int max_size = 65;
    char str[max_size];
    memset(str, '\0', max_size);

    for(long i = 0; i < rems.size(); i ++)
        if( N != 26 )
            str[i] = digits[ rems[i] ];
        else
            str[i] = digits[ rems[i] ];
    std::string s( str, 0, rems.size() );

    if(!flag)
        s = std::string( "-" ) + s;
    return s;
}

public:
std::string base26(long v)
{
    bool flag = v >= 0;
    if(!flag)
        v = -v;

    char const digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    size_t const base = sizeof(digits) - 1;
    char result[sizeof(unsigned long)*CHAR_BIT + 1];
    char* current = result + sizeof(result);
    *--current = '\0';

    while (v != 0)
    {
        v--;
        *--current = digits[v % base];
        v /= base;
    }

    std::string s(current);
    if(!flag)
        s = std::string( "-" ) + s;

    return s;
}

std::string base2 (long v) { return base_conversion<2> (v); }
std::string base8 (long v) { return base_conversion<8> (v); }
std::string base10(long v) { return base_conversion<10>(v); }
std::string base16(long v) { return base_conversion<16>(v); }
std::string base36(long v) { return base_conversion<36>(v); }

};

/*
template<class T>
std::string printf_format(T x)
{
    std::map<const char*, std::string> typenames;

    typenames[typeid(short).name()] = "%d";
    typenames[typeid(unsigned short).name()] = "%u";

    typenames[typeid(int).name()] = "%d";
    typenames[typeid(unsigned int).name()] = "%u";

    typenames[typeid(long).name()] = "%ld";
    typenames[typeid(unsigned long).name()] = "%lu";

    typenames[typeid(long long).name()] = "%lld";
    typenames[typeid(unsigned long long).name()] = "%llu";

    typenames[typeid(float).name()] = "%f";
    typenames[typeid(double).name()] = "%f";
    //typenames[typeid(std::complex<double>).name()] = "(%.2f,%.2f)";
    typenames[typeid(std::complex<double>).name()] = "(%.0f,%.0f)";

    typenames[typeid(long double).name()] = "%lf";

    typenames[typeid(bool).name()] = "%d";

    typenames[typeid(char).name()] = "%c";
    typenames[typeid(signed char).name()] = "%c";
    typenames[typeid(unsigned char).name()] = "%c";

    typenames[typeid(std::string).name()] = "%s";

    std::map<const char*, std::string>::iterator it = typenames.find( typeid(x).name() );
    if(it != typenames.end())
    {
        return it->second;
    }
    else
    {
        std::cout<< coerr << "Type not implemented" << codef << std::endl;
        std::cout<< coerr << "Aborted" << codef << std::endl;
        mars_log( typeid(x).name() );
        //mars_log(x);
        exit( EXIT_FAILURE );
    }
}
*/

/*
std::string to_string_short(const double __x)
{
    double x;
    if (__x < 0)
        x = fabs( __x );
    else if(__x == 0)
        return  "0";
    else
        x = __x;

    std::stringstream ssx;
    std::string stx;
    ssx <<x;
    if(ssx.str().size() > 4 )
    {
        double order = floor( log10(fabs(x)) );
        int sig = x / pow(10.0, order);
        stx = to_string(sig) + "E" + to_string(order);
    }
    else
    {
        stx = ssx.str();
    }

    if(__x < 0)
        stx = std::string("-") + stx;

    return  stx;
}
*/

//std::string  get_programe_name()
//{
//    std::string  str = std::string( program_invocation_name );
//    unsigned found = str.find_last_of("/\\");
//    //std::cout<< "Splitting: " << str << '\n';
//    //std::cout<< " path: " << str.substr(0,found) << '\n';
//    //std::cout<< " file: " << str.substr(found+1) << '\n';
//    return  str.substr(found+1);
//}

/*
std::string stfrin ( in  n )
{
    ch* s  = new ch[17];
    st  u;

	if( n < 0 )
	{                   //  turns n positive
		n = -n;
		u = "-";        //  adds '-' on res string
	}

    in  i  = 0;         //  s counter
    do
    {
        s[i ++]= n%10 + '0'; //  conversion of each digit of n to char
        n -= n%10;          //  update n value
    }
    while ( ( n /= 10 ) > 0 );

    for( in  j = i - 1; j >= 0; -- j )
    {
        u += s[j];          //  building our st number
    }

    delete [] s;            //  free-up the memory!

    return  u;
}
*/

// comparison, not case sensitive.
struct StringCompareCase
{
    bool operator() ( const std::string& lhs, const std::string& rhs )
    {
        std::string::const_iterator lb = lhs.begin(), le = lhs.end(), rb = rhs.begin(), re = rhs.end();
        char lc, rc;
        for( ; lb != le && rb != re; ++lb, ++rb)
        {
            lc = char(tolower(*lb));
            rc = char(tolower(*rb));
            if (lc < rc)
                return  true;
            if (lc > rc)
                return  false;
        }
        // if rhs is longer than lhs then lhs<rhs
        return (rb != re);
    }
};

struct StringCompareICase
{
    bool operator()( const std::string& first, const std::string& second )
    {
        unsigned int i=0;
        while ( (i<first.length()) && (i<second.length()) )
        {
            if ( tolower(first[i]) < tolower(second[i]) )
                return true;
            else if ( tolower(first[i]) > tolower(second[i]) )
                return false;
            ++ i;
        }

        if (first.length()<second.length())
            return true;
        else
            return false;
    }
};




}

#endif
