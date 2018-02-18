Array: C++ multidimensional array with functionalities: supporting slice in place, operations on sliced array in place, map in place, reshape, transpose, iterator (STL algorithms can be used on the array, for example sorting in place), sum by axes. Optimized for speed. Currently max dimension is 6.

Required packages: pthread, libunwind for debugging, GSL for random numbers (https://www.gnu.org/software/gsl/), FFTW for Fourier transform (http://www.fftw.org/)

Compile and run:
Array testing: g++ -std=c++14 test_array.cc -o test_array.x -lpthread -lunwind -lgsl -lgslcblas; ./test_array.x

Test run results are in test_array.txt

Benchmarked Mars for 3 dimensional array of double precision. Most times the speed is the same as simple C array declared on stack. Sometimes, Mars is even faster.
