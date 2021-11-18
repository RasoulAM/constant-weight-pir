#include "client.h"

Client::Client(){
    this->server_ = new PIRserver;
    this->final_noise_level = vector<uint64_t>();
}

void Client::set_server(PIRserver* _server){
    this->server_ = _server;
}

void Client::setup_crypto(size_t log_poly_modulus_degree, uint64_t prime_bitlength, bool _verbose){

    /* Parameter Selection */
    this->parms = new EncryptionParameters(scheme_type::bfv);
    this->parms->set_poly_modulus_degree(1<<log_poly_modulus_degree);
    this->log_poly_mod_degree = log_poly_modulus_degree;
    this->parms->set_coeff_modulus(CoeffModulus::BFVDefault(1<<log_poly_modulus_degree));

    uint64_t coeff_bitcount = 0;
    for (auto &prime : this->parms->coeff_modulus()){
        coeff_bitcount += prime.bit_count();
    }
    
    // uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
    // Modulus mod(_prime);
    // this->parms->set_plain_modulus(mod);
    // uint64_t prime_bitlength = 21;
    this->parms->set_plain_modulus(PlainModulus::Batching(1<<log_poly_mod_degree, prime_bitlength));
    this->coefficient_hex_length = prime_bitlength / 4;
    this->parms->save(parms_stream);

    if (_verbose){
        cout << "-------------------------- Crypto Parameters --------------------------" << endl
            << "\tPoly Mod Degree: " << this->parms->poly_modulus_degree() << endl
            << "\tPlain Modulus: " << this->parms->plain_modulus().value() << " (" << this->parms->plain_modulus().bit_count() << " bits)" << endl
            << "\tCiphertext Modulus Bitcount: " << coeff_bitcount << endl;
    }

    this->context = new SEALContext(*(this->parms));
    this->keygen = new KeyGenerator(*(this->context));
    this->sk = keygen->secret_key();

    this->evaluator = new Evaluator(*(this->context));

}

void Client::send_evaluation_keys(){
    vector<uint32_t> elts;
    for (int i=0;i< this->log_poly_mod_degree;i++){
        elts.push_back((1<<(i+1))+1);
    }
    Serializable<RelinKeys> rlk_client = keygen->create_relin_keys();
    this->comm_relin += rlk_client.save(data_stream);
    Serializable<GaloisKeys> gal_keys_client = keygen->create_galois_keys(elts);
    this->comm_gals += gal_keys_client.save(data_stream);

    this->sk.save(sk_stream);
}

// Common for all
vector<uint64_t> Client::query_to_encoding(uint64_t _query, QueryParameters* q_params){
    vector<uint64_t> _ans;
    switch (q_params->eq_type) {
        case Constant_Weight:
            _ans = get_perfect_constant_weight_codeword(_query, q_params->encoding_size, q_params->hamming_weight);
            break;
        case Folklore:
            _ans = get_folklore_encoding(_query, q_params->encoding_size, q_params->hamming_weight);
            break;
    }
    return _ans;
}

vector<Plaintext> Client::encoding_to_pt(QueryParameters* q_params, vector<uint64_t> enc_){
    string __query_string;
    string hex_inverse = int_to_hex(prime_mod_inverse(q_params->used_slots_per_pt, this->parms->plain_modulus().value()));
    vector<Plaintext> __query_plaintexts;
    for (int i=0;i<q_params->num_input_ciphers;i++){
        __query_string = "";
        for (size_t j = 0; j < q_params->used_slots_per_pt; j++){
            if (i*q_params->used_slots_per_pt+j < q_params->encoding_size && enc_[i*q_params->used_slots_per_pt+j] == 1ULL){
                __query_string = hex_inverse +  ((j==0)?"":"x^"+to_string(j)) + ((__query_string.empty())?"":" + ") + __query_string;
            }
        }
        if (__query_string.empty()) __query_string = "0";

        __query_plaintexts.push_back(Plaintext(__query_string));
    }
    return __query_plaintexts;
}

void Client::encrypt_and_send(vector<Plaintext>& pts, uint64_t num_pts){
    Encryptor sym_encryptor(*(this->context), this->sk);

    for (int i=0;i<num_pts;i++){
        Serializable<Ciphertext> __sym_encrypted_query = sym_encryptor.encrypt_symmetric(pts[i]);
        this->comm_query += __sym_encrypted_query.save(data_stream);
    }
}

vector<Plaintext> Client::load_and_decrypt(QueryParameters* q_params){
    Decryptor decryptor(*(this->context), sk);
    vector<Plaintext> encrypted_result(q_params->num_output_ciphers);
    Ciphertext __temp_ct;
    for (int i=0;i<q_params->num_output_ciphers;i++){
        this->comm_response += __temp_ct.load(*(this->context), data_stream);
        this->final_noise_level.push_back(decryptor.invariant_noise_budget(__temp_ct));
        decryptor.decrypt(__temp_ct, encrypted_result[i]);
    }
    return encrypted_result;
}

vector<uint64_t> Client::pts_to_vec(vector<Plaintext>& _pts, uint64_t num_pts_, uint64_t poly_degree){
    vector<uint64_t> _res(0);
    for (int i=0;i<num_pts_; i++){
        vector<uint64_t> __temp = poly_string_to_hex_vector(_pts[i].to_string(), poly_degree);
        _res.insert( _res.end(), __temp.begin(), __temp.end() );
    }
    return _res;
}

void Client::submit_PIR_query_with_params(uint64_t _query, QueryParameters* _q_params, bool _verbose){
    chrono::high_resolution_clock::time_point time_start_local, time_end_local;
    chrono::microseconds time_diff_local;

    time_start_local = chrono::high_resolution_clock::now();
        vector<uint64_t> query_encoding = query_to_encoding(_query, _q_params);
        vector<Plaintext> __query_plaintexts = encoding_to_pt(_q_params, query_encoding);
        encrypt_and_send(__query_plaintexts, _q_params->num_input_ciphers);
    time_end_local = chrono::high_resolution_clock::now();
    time_diff_local = chrono::duration_cast<chrono::microseconds>(time_end_local - time_start_local);
    _q_params->metrics_["time_query"] = this->time_diff.count();

}

uint64_t Client::submit_random_PIR_query_with_params(QueryParameters* _q_params, bool _verbose){
    uint64_t __query = _q_params->db_->get_random_query();
    this->submit_PIR_query_with_params(__query, _q_params, _verbose);
    return __query;
}

bool Client::validate_response(QueryParameters* q_params, uint64_t _query_keyword, vector<uint64_t> _response, bool _verbose){
    bool __equal = true;
    vector<string> coeffs = q_params->db_->get_hex_data_by_keyword(_query_keyword, this->coefficient_hex_length);
    for (uint64_t i=0;i<coeffs.size();i++){
        if (_response[i] != hex_string_to_uint(coeffs[i])){
            __equal = false;
        }
    }
    if (_verbose){
        if (__equal){
                cout << "Correct Response!" << endl;
        } else {
                cout << "Incorrect Reponse!" << endl;
        }
    }
    return __equal;
}

// End to End
bool Client::end_to_end_PIR_query(QueryParameters* _q_params, bool _verbose){

    if (_verbose) cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    this->time_start = chrono::high_resolution_clock::now();
        this->setup_crypto(_q_params->log_poly_mod_degree, _q_params->prime_bitlength, _verbose);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    _q_params->metrics_["time_setup_crypto"] = this->time_diff.count();

    if (_verbose) _q_params->print_db_specific_parameters();

    // Setting up the server
    this->server_->set_params(parms_stream, sk_stream, _verbose);
    this->server_->prep_db(_q_params, _verbose);
    this->send_evaluation_keys();

    uint64_t _query = this->submit_random_PIR_query_with_params(_q_params, _verbose);

    // Invoking server
    this->time_start = chrono::high_resolution_clock::now();
        this->server_->respond_pir_query(data_stream, _q_params, _verbose);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    _q_params->metrics_["time_server_latency"] = this->time_diff.count();

    // Extracting Response
    this->time_start = chrono::high_resolution_clock::now();
        vector<Plaintext> _response_pts = this->load_and_decrypt(_q_params);
        vector<uint64_t> _response_vec = this->pts_to_vec(_response_pts, _q_params->num_output_ciphers, _q_params->poly_mod_degree);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    _q_params->metrics_["time_extract_response"] = this->time_diff.count();
    
    // Validating Response
    bool _check = this->validate_response(_q_params, _query, _response_vec, _verbose);

    if (!_check){
        cout << "Noise Level: " << *min_element(begin(final_noise_level), end(final_noise_level)) << endl;
    }

    _q_params->metrics_["comm_relin"] = comm_relin;
    _q_params->metrics_["comm_gals"] = comm_gals;
    _q_params->metrics_["comm_query"] = comm_query;
    _q_params->metrics_["comm_response"] = comm_response;
    _q_params->metrics_["valid_response"] = _check;
    _q_params->add_parameters_to_metrics();

    // Printing timings
    if (_verbose){
        cout << "------------------------ Timing ----------------------------------------------" << endl;
        cout << "\tDatabase Prep   : " << setw(10) << _q_params->metrics_["time_prep"]/1000 << " ms" << endl;
        cout << "\tExpansion time: : " << setw(10) << _q_params->metrics_["time_expansion"]/1000 << " ms" << endl;
        cout << "\tSel. Vec. Calc. : " << setw(10) << _q_params->metrics_["time_sel_vec"]/1000 << " ms" << endl;
        cout << "\tInner Product   : " << setw(10) << _q_params->metrics_["time_inner_prod"]/1000 << " ms" << endl;
        cout << "\tTotal Server    : " << setw(10) << _q_params->metrics_["time_server_total"]/1000 << " ms" << endl;
        cout << "--------------------- Communication ------------------------------------------" << endl
            << "\tData Independant: " << comm_relin/1000 << " KB (Relin keys) + " << comm_gals/1000 << " KB (Gal Keys)" << endl
            << "\tData Dependant: " << comm_query/1000 << " KB (Query) + " << comm_response/1000 << " KB (Reponse)" << endl << endl
            << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;
    }

    if (_q_params->write_path != "") _q_params->write_results_to_file();
    
    return _check;
}