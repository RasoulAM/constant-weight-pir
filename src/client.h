#ifndef __CLIENT__H
#define __CLIENT__H

#include "utils.h"
#include "pirutils.h"
#include "PIRserver.h"

using namespace std;
using namespace seal;

class Client{

    public:
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff;

    EncryptionParameters* parms;
    uint64_t log_poly_mod_degree;
    uint64_t coefficient_hex_length;
    SEALContext* context;
    KeyGenerator* keygen;
    SecretKey sk;
    Evaluator* evaluator;

    stringstream parms_stream;
    stringstream data_stream;
    stringstream sk_stream;

    PIRserver* server_;
    vector<uint64_t> final_noise_level;

    long comm_relin=0, comm_gals=0, comm_query=0, comm_response=0;

    // Preparation steps

    Client();

    void set_server(PIRserver* _server);
    void setup_crypto(size_t log_poly_modulus_degree = 13, uint64_t prime_bitlength = 21, bool _verbose=true);
    void send_evaluation_keys();
    vector<uint64_t> query_to_encoding(uint64_t _query, QueryParameters* q_params);
    vector<Plaintext> encoding_to_pt(QueryParameters* q_params, vector<uint64_t> enc_);
    void encrypt_and_send(vector<Plaintext>& pts, uint64_t num_pts);
    vector<Plaintext> load_and_decrypt(QueryParameters* q_params);
    vector<uint64_t> pts_to_vec(vector<Plaintext>& _pts, uint64_t num_pts_, uint64_t poly_degree);
    void submit_PIR_query_with_params(uint64_t _query, QueryParameters* _q_params, bool _verbose=true);
    uint64_t submit_random_PIR_query_with_params(QueryParameters* _q_params, bool _verbose=true);
    bool validate_response(QueryParameters* q_params, uint64_t _query_keyword, vector<uint64_t> _response, bool _verbose=true);
    // End to End
    bool end_to_end_PIR_query(QueryParameters* _q_params, bool _verbose = true);
};

#endif