#ifndef __UTILS__H
#define __UTILS__H

// #include "utils.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <time.h>
#include <omp.h>

using namespace std;

/*
Helper function: Print line number.
*/
inline void print_line(int line_number){
    std::cout << "Line " << std::setw(3) << line_number << " --> ";
}

/*
Helper function: Prints a matrix of values.
*/
template <typename T>
inline void print_matrix(std::vector<T> matrix, std::size_t row_size){
    /*
    We're not going to print every column of the matrix (there are 2048). Instead
    print this many slots from beginning and end of the matrix.
    */
    std::size_t print_size = 500;

    std::cout << std::endl;
    std::cout << "    [";
    for (std::size_t i = 0; i < print_size; i++)
    {
        std::cout << std::setw(3) << std::right << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = row_size - print_size; i < row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
    }
    std::cout << "    [";
    for (std::size_t i = row_size; i < row_size + print_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ",";
    }
    std::cout << std::setw(3) << " ...,";
    for (std::size_t i = 2 * row_size - print_size; i < 2 * row_size; i++)
    {
        std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
    }
    std::cout << std::endl;
}

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << std::hex << i;
  return stream.str();
}

string gen_random_hex();
string gen_random_hex(int length);
string gen_random_hex(int length, int x);
std::string vector_to_poly_string(vector<uint64_t> coeffs);
std::string hex_vector_to_poly_string(vector<string> coeffs);
uint64_t hex_string_to_uint(string str);
pair<uint64_t, string> monomial_to_hex(string monomial);
pair<uint64_t, uint64_t> monomial_to_integer(string monomial);
vector<uint64_t> poly_string_to_vector(std::string poly_string, uint64_t max_size);
vector<uint64_t> poly_string_to_hex_vector(std::string poly_string, uint64_t max_size);
uint64_t mod_exp(uint64_t a, uint64_t e, uint64_t n);
uint64_t prime_mod_inverse(uint64_t a, uint64_t n);
uint64_t iter_factorial(uint64_t n);
uint64_t choose(uint64_t n, uint64_t k);
uint64_t find_choose(uint64_t k, uint64_t n);
float log2_choose(uint64_t n, uint64_t k);
uint64_t find_log2_choose(uint64_t k, uint64_t log_n);
uint64_t bigger_power2(uint64_t x);
std::vector<uint64_t> get_perfect_constant_weight_codeword(uint64_t __number, uint64_t encoding_size, uint64_t hamming_weight, bool __verbose=true);
std::vector<uint64_t> get_folklore_encoding(uint64_t __number, uint64_t encoding_size, uint64_t hamming_weight, bool __verbose=true);

#endif