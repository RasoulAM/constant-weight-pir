#ifndef __PIRSERVER__H
#define __PIRSERVER__H

#include "utils.h"
#include "database.h"
#include "pirutils.h"
#include "seal/seal.h"
#include "seal/util/polyarithsmallmod.h"
#include <thread>

using namespace std;
using namespace seal;

class PIRserver{

    public:
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;

    seal::EncryptionParameters* parms;
    seal::SEALContext* context;

    Evaluator* evaluator;
    RelinKeys* rlk_server;
    GaloisKeys* gal_keys_server;

    Encryptor* enc;
    Decryptor* noise_calculator;

    PIRserver();
    void initialize(uint64_t poly_modulus_degree=8192);
    void initialize_params_with_input(stringstream &parms_stream);
    void load_keys(stringstream &data_stream);
    void set_parameters_used_for_debug(stringstream &sk_stream);
    void prep_db(QueryParameters* q_params, bool _verbose=true);
    void multiply_inverse_power_of_x(const seal::Ciphertext& encrypted, uint32_t k, seal::Ciphertext& destination);
    vector<Ciphertext> expand_procedure(const Ciphertext &input_cipher, uint64_t used_slots);
    vector<Ciphertext> expand_input_ciphers(vector<Ciphertext>& input_ciphers, uint64_t num_input_ciphers, uint64_t num_bits);
    Ciphertext constant_weight_eq(QueryParameters* q_params, vector<Ciphertext>& encrypted_query, vector<uint64_t>& __encoded_subindex);
    Ciphertext folklore_eq(QueryParameters* q_params, vector<Ciphertext>& encrypted_query, vector<uint64_t>& __encoded_subindex);
    Ciphertext generate_selection_bit(QueryParameters* q_params, uint64_t _keyword, vector<Ciphertext>& encrypted_query);
    void generate_selection_vector(QueryParameters* q_params, vector<Ciphertext>& expanded_query,  vector<Ciphertext>& selection_vector, bool _verbose);
    vector<Ciphertext> multiply_with_database(QueryParameters* q_params, uint64_t _subindex, Ciphertext& encrypted_selection_bit);
    void inner_product(QueryParameters* q_params, vector<Ciphertext>& selection_vector, vector<Ciphertext>& encrypted_answer, bool _verbose);
    void faster_inner_product(QueryParameters* q_params, vector<Ciphertext>& selection_vector, vector<Ciphertext>& encrypted_answer, bool _verbose);
    void set_params(stringstream &parms_stream, stringstream &sk_stream, bool _verbose=true);
    void respond_pir_query(stringstream &data_stream, QueryParameters *q_params, bool _verbose=true);

};

#endif