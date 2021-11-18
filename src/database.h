#ifndef __DATABASE__H
#define __DATABASE__H

#include "utils.h"
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

class Database{

    public:

    uint64_t max_item_hex_length;
    uint64_t num_keywords;

    vector<uint64_t> keywords;
    vector<string> data_hex_raw;

    uint64_t MAX_KEYWORD_BITLENGTH;

    Database(uint64_t __num_keywords, uint64_t __max_item_hex_length, uint64_t __MAX_KEYWORD_BITLENGTH);
    void generate_some_hex_data();
    uint64_t get_random_query();
    uint64_t get_index_of_keyword(uint64_t keyword);
    vector<string> get_hex_data_by_keyword(uint64_t keyword, uint64_t coefficient_hexlength);
    vector<string> get_hex_data_by_keyword_index(uint64_t keyword_index, uint64_t coefficient_hexlength);



};

#endif