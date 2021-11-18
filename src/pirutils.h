#ifndef __PIRUTILS_H
#define __PIRUTILS_H

#include <map>
#include "database.h"
#include "utils.h"
#include "seal/seal.h"
using namespace seal;

enum EqualityType {
    Folklore,
    Constant_Weight
};


class QueryParameters{
    public:

    // Reshaped database parameters
    uint64_t log_poly_mod_degree;
    uint64_t poly_mod_degree;
    uint64_t prime_bitlength=21;
    uint64_t coefficient_hexlength;
    uint64_t num_output_ciphers;
    uint64_t keyword_bitlength;

    // Per-query parameters
    uint64_t hamming_weight;
    EqualityType eq_type;
    uint64_t compression_factor;
    uint64_t encoding_size;
    uint64_t ceil_log2_encoding_size;
    uint64_t used_slots_per_pt;
    uint64_t num_input_ciphers;


    Database* db_;
    vector<Plaintext>* coefficient_vector_pt;

    map<string, uint64_t> metrics_;
    string write_path="";
    uint64_t num_threads=0;

    QueryParameters(uint64_t log_poly_mod_degree, EqualityType eq_type, uint64_t hamming_weight, Database* db, uint64_t keyword_bitlength=0, uint64_t num_threads=0);
    void init(uint64_t log_poly_mod_degree, EqualityType eq_type, uint64_t hamming_weight, Database* db_, uint64_t num_threads=0);
    void print_db_specific_parameters();
    void add_parameters_to_metrics();
    void write_results_to_file();
    void set_db_parameters();
    void set_query_specific_parameters();
    vector<uint64_t> get_encoding_of_(uint64_t __keyword);
};

#endif
