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

#ifndef MARS_HEADER_H
#define MARS_HEADER_H 1

#include <iostream>
#include <cstring>
#include <math.h>
#include <inttypes.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <iomanip>
#include <iterator>
#include <chrono>
#include <typeinfo>
#include <type_traits>
#include <complex>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <csignal>
#include <ctime>
#include <stdarg.h>
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <list>
#include <set>
#include <utility>
#include <ctype.h>
#include <deque>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <limits.h>

#include <new>
#include <locale>

#define UNW_LOCAL_ONLY

//#include <thread>
#include <cxxabi.h>

#include <dirent.h>
#include <errno.h>
#include <exception>
#include <execinfo.h>
#include <fcntl.h>

#include <libgen.h>
#include <libunwind.h>

#include <regex>
#include <semaphore.h>

#include <signal.h>

#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include <unistd.h>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_statistics.h>

#include <fftw3.h>

#include <boost/range/irange.hpp>
#include <boost/functional/hash.hpp> /* hash std::pair<long,long> */

#define MARS_COLOR_MODE   1
#ifdef  MARS_COLOR_MODE

#define MARS_COLOR_RED     "\033[22;31m"
#define MARS_COLOR_GREEN   "\033[22;32m"
#define MARS_COLOR_YELLOW  "\033[22;33m"
#define MARS_COLOR_BLUE    "\033[22;34m"
#define MARS_COLOR_MAGENTA "\033[22;35m"
//#define MARS_COLOR_CYAN    "\033[22;36m"
#define MARS_COLOR_CYAN    "\e[1;36m" // bright cyan
#define MARS_COLOR_WHITE   "\e[1;37m"

#define codef          "\033[0;22m"
#else
#define MARS_COLOR_RED     ""
#define MARS_COLOR_GREEN   ""
#define MARS_COLOR_YELLOW  ""
#define MARS_COLOR_BLUE    ""
#define MARS_COLOR_MAGENTA ""
#define MARS_COLOR_CYAN    ""

#define CODEF         ""
#endif

#define COFUNC   MARS_COLOR_CYAN
#define COFILE   MARS_COLOR_CYAN
#define COFOLDER MARS_COLOR_BLUE
#define COVAR    MARS_COLOR_RED
#define COVAL    MARS_COLOR_WHITE
#define COERR    MARS_COLOR_RED
#define COWARN   MARS_COLOR_YELLOW

#if __STDC_VERSION__ < 199901L
#if __GNUC__ >= 2
#define __FCN__ __PRETTY_FUNCTION__
#else
#define __FCN__ __func__
#endif
#endif

#endif
