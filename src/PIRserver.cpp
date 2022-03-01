#include "PIRserver.h"

PIRserver::PIRserver(){

}

void PIRserver::initialize(uint64_t poly_modulus_degree){
    this->parms=new EncryptionParameters(scheme_type::bfv);
    this->parms->set_poly_modulus_degree(poly_modulus_degree);
    this->parms->set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    this->parms->set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 21));
    
    this->context = new SEALContext(*(this->parms));
    KeyGenerator keygen(*context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    evaluator = new Evaluator(*context);
    this->rlk_server = new RelinKeys();
    keygen.create_relin_keys(*(this->rlk_server));
    
    vector<uint32_t> elts;
    for (int i=1;i< poly_modulus_degree;i*=2){
        elts.push_back(2*i+1);
    }
    this->gal_keys_server = new GaloisKeys();
    keygen.create_galois_keys(elts, *(this->gal_keys_server));

    this->enc = new Encryptor(*context, pk);
    this->noise_calculator = new Decryptor(*context, sk);

}

void PIRserver::initialize_params_with_input(stringstream &parms_stream){
    this->parms=new EncryptionParameters();
    this->parms->load(parms_stream);

    parms_stream.seekg(0, parms_stream.beg);
    context = new SEALContext(*(this->parms));

    this->evaluator = new Evaluator(*context);

}

void PIRserver::load_keys(stringstream &data_stream){
    this->rlk_server=new RelinKeys();
    this->gal_keys_server=new GaloisKeys();
    this->rlk_server->load(*context, data_stream);
    this->gal_keys_server->load(*context, data_stream);
}

void PIRserver::set_parameters_used_for_debug(stringstream &sk_stream){
    SecretKey sk;
    sk.load(*context, sk_stream);
    noise_calculator=new Decryptor(*context, sk);
    KeyGenerator keygen(*context, sk);

    PublicKey pk;
    keygen.create_public_key(pk);
    this->enc = new Encryptor(*context, pk);
}

void PIRserver::prep_db(QueryParameters* q_params, bool _verbose){
    this->time_start = chrono::high_resolution_clock::now();
    
    q_params->coefficient_vector_pt = new vector<Plaintext>[q_params->db_->num_keywords];

    #pragma omp parallel
    {
        #pragma omp for
        for (uint64_t i=0;i<q_params->db_->num_keywords;i++){
            q_params->coefficient_vector_pt[i] = vector<Plaintext>();
            vector<string> __fetched_db_row = q_params->db_->get_hex_data_by_keyword_index(i,q_params->coefficient_hexlength);

            vector<string>::iterator __start, __end;
            for (uint64_t j=0;j<q_params->num_output_ciphers;j++){
                __start = __fetched_db_row.begin() + j * q_params->poly_mod_degree;
                __end = min(__fetched_db_row.begin() + (j+1) * q_params->poly_mod_degree, __fetched_db_row.end());
                vector<string> __temp2(__start, __end);
                string poly_string = hex_vector_to_poly_string(__temp2);
                Plaintext _pt(poly_string);
                this->evaluator->transform_to_ntt_inplace(_pt, this->context->first_parms_id());
                q_params->coefficient_vector_pt[i].push_back(_pt);
            }
            __fetched_db_row.clear();
        }
    }

    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);
    q_params->metrics_["time_prep"] = this->time_diff.count();
}

void PIRserver::multiply_inverse_power_of_x(
    const seal::Ciphertext& encrypted, uint32_t k,
    seal::Ciphertext& destination) {

    const auto& params = this->context->first_context_data()->parms();
    const auto poly_modulus_degree = params.poly_modulus_degree();
    const auto coeff_mod_count = params.coeff_modulus().size();


    uint32_t index =
        ((poly_modulus_degree << 1) - k) % (poly_modulus_degree << 1);

    destination = encrypted;

    for (size_t i = 0; i < encrypted.size(); i++) {
        for (size_t j = 0; j < coeff_mod_count; j++) {
        seal::util::negacyclic_shift_poly_coeffmod(
            encrypted.data(i) + (j * poly_modulus_degree), poly_modulus_degree,
            index, params.coeff_modulus()[j],
            destination.data(i) + (j * poly_modulus_degree));
        }
    }
}

vector<Ciphertext> PIRserver::expand_procedure(const Ciphertext &input_cipher, uint64_t used_slots){

    vector<Ciphertext> ciphers(used_slots);
    uint64_t expansion_level = (int)ceil(log2(used_slots));
    ciphers[0] = input_cipher;
    for (uint64_t a = 0; a < expansion_level; a++){
        #pragma omp parallel
        {
            #pragma omp for
            for (uint64_t b = 0; b < (1<<a); b++){
                auto __temp_0=ciphers[b];
                auto __temp_2=ciphers[b];
                evaluator->apply_galois_inplace(__temp_0, (parms->poly_modulus_degree() >> a) + 1, *gal_keys_server);
                evaluator->add_inplace(ciphers[b], __temp_0);

                if (b+(1<<a) < used_slots){
                    Ciphertext __temp_1;
                    this->multiply_inverse_power_of_x(__temp_2, 1<<a, ciphers[b+(1<<a)]);
                    this->multiply_inverse_power_of_x(__temp_0, 1<<a, __temp_1);
                    evaluator->sub_inplace(ciphers[b+(1<<a)], __temp_1);
                }
            }
        }
    
    }
    return ciphers;
}

vector<Ciphertext> PIRserver::expand_input_ciphers(vector<Ciphertext>& input_ciphers, uint64_t num_input_ciphers, uint64_t num_bits){
    vector<Ciphertext> __answer;
    vector<Ciphertext> __temp_expanded;
    uint64_t remaining_bits = num_bits;
    for (int i=0;i<num_input_ciphers;i++){
        __temp_expanded.clear();
        __temp_expanded = expand_procedure(input_ciphers[i], min(remaining_bits, parms->poly_modulus_degree()));
        for (int j=0;j<__temp_expanded.size();j++)
            __answer.push_back(__temp_expanded[j]);
        remaining_bits -= __temp_expanded.size();
    }
    return __answer;
}

Ciphertext PIRserver::constant_weight_eq(QueryParameters* q_params, vector<Ciphertext>& encrypted_query, vector<uint64_t>& __encoded_subindex){
    Ciphertext __temp_ciphertext;
    if (q_params->hamming_weight > 1){
        vector<Ciphertext> mult_operands;
        for (uint64_t i=0;i<q_params->encoding_size;i++){
            if (__encoded_subindex[i]==1){
                mult_operands.push_back(encrypted_query[i]);
            }
        }
        evaluator->multiply_many(mult_operands, *rlk_server, __temp_ciphertext);
    } else {
        for (uint64_t i=0;i<q_params->encoding_size;i++){
            if (__encoded_subindex[i]==1){
                __temp_ciphertext = encrypted_query[i];
            }
        }
    }
    return __temp_ciphertext;
}

Ciphertext PIRserver::folklore_eq(QueryParameters* q_params, vector<Ciphertext>& encrypted_query, vector<uint64_t>& __encoded_subindex){
    Ciphertext __temp_ciphertext;
    vector<Ciphertext> mult_operands;
    for (uint64_t i=0;i<q_params->encoding_size;i++){
        if (__encoded_subindex[i]==1){
            mult_operands.push_back(encrypted_query[i]);
        } else {
            Ciphertext _operand;
            evaluator->sub_plain(encrypted_query[i], Plaintext("1"), _operand);
            evaluator->negate_inplace(_operand);
            mult_operands.push_back(_operand);
        }
    }
    evaluator->multiply_many(mult_operands, *rlk_server, __temp_ciphertext);
    return __temp_ciphertext;
}

Ciphertext PIRserver::generate_selection_bit(QueryParameters* q_params, uint64_t _keyword, vector<Ciphertext>& encrypted_query){
    vector<uint64_t> __encoded_subindex;

    __encoded_subindex = q_params->get_encoding_of_(_keyword);

    Ciphertext __temp_ciphertext;

    switch (q_params->eq_type) {
        case Folklore:                
            __temp_ciphertext = folklore_eq(q_params, encrypted_query, __encoded_subindex);
            break;
        case Constant_Weight:
            __temp_ciphertext = constant_weight_eq(q_params, encrypted_query, __encoded_subindex);
            break;
    }

    return __temp_ciphertext;

}

void PIRserver::generate_selection_vector(QueryParameters* q_params, vector<Ciphertext>& expanded_query,  vector<Ciphertext>& selection_vector, bool _verbose){
    
    #pragma omp parallel for
    for (uint64_t ch=0;ch<q_params->db_->num_keywords;ch++){
        selection_vector[ch] = generate_selection_bit(q_params, q_params->db_->keywords[ch], expanded_query);
    }
    return;

}

vector<Ciphertext> PIRserver::multiply_with_database(QueryParameters* q_params, uint64_t _subindex, Ciphertext& encrypted_selection_bit){

    vector<Ciphertext> __answer(q_params->num_output_ciphers);

    this->evaluator->transform_to_ntt_inplace(encrypted_selection_bit);

    for (uint64_t o=0;o<q_params->num_output_ciphers;o++){
        evaluator->multiply_plain(encrypted_selection_bit, q_params->coefficient_vector_pt[_subindex][o], __answer[o]);
    }
    return __answer;        
}

void PIRserver::inner_product(QueryParameters* q_params, vector<Ciphertext>& selection_vector, vector<Ciphertext>& encrypted_answer, bool _verbose){
    
    vector<Ciphertext>* __ciphers = new vector<Ciphertext>[q_params->num_output_ciphers];
    #pragma omp parallel for
    for (uint64_t ch=0;ch<q_params->db_->num_keywords;ch++){
        vector<Ciphertext> encrypted_compare;
        encrypted_compare= multiply_with_database(q_params, ch, selection_vector[ch]);
        #pragma omp critical
        {
            for (int o=0;o<q_params->num_output_ciphers;o++)
                __ciphers[o].push_back(encrypted_compare[o]);
        }
    }

    for (uint64_t o = 0; o < q_params->num_output_ciphers; o++){
        evaluator->add_many(__ciphers[o], encrypted_answer[o]);
    }        
    return;
}

void PIRserver::faster_inner_product(QueryParameters* q_params, vector<Ciphertext>& selection_vector, vector<Ciphertext>& encrypted_answer, bool _verbose){

    #pragma omp parallel for
    for (uint64_t i=0;i<selection_vector.size();i++){
        this->evaluator->transform_to_ntt_inplace(selection_vector[i]);
    } 

    vector<Ciphertext>* __sub_ciphers = new vector<Ciphertext>[q_params->num_output_ciphers];

    #pragma omp parallel
    {
        Ciphertext __operand;
        #pragma omp for collapse(2)
        for (uint64_t ch=0;ch<q_params->db_->num_keywords;ch++){
            for (uint64_t o=0;o<q_params->num_output_ciphers;o++){
                this->evaluator->multiply_plain(selection_vector[ch], q_params->coefficient_vector_pt[ch][o], __operand);
                #pragma omp critical
                {
                    __sub_ciphers[o].push_back(__operand);
                }
            }
        }

    }

    #pragma omp parallel for
    for (uint64_t o = 0; o < q_params->num_output_ciphers; o++){
        this->evaluator->add_many(__sub_ciphers[o], encrypted_answer[o]);
    }

    return;

}

void PIRserver::set_params(stringstream &parms_stream, stringstream &sk_stream, bool _verbose){
    this->initialize_params_with_input(parms_stream);
    bool DEBUG = false;
    if (DEBUG)
        this->set_parameters_used_for_debug(sk_stream);
}

void PIRserver::respond_pir_query(stringstream &data_stream, QueryParameters *q_params, bool _verbose){
    chrono::high_resolution_clock::time_point time_start_local, time_end_local;
    chrono::microseconds time_diff_local;

    this->load_keys(data_stream);

    time_start_local = chrono::high_resolution_clock::now();

    vector<Ciphertext> encrypted_query(q_params->num_input_ciphers);
    vector<Ciphertext> encrypted_answer(q_params->num_output_ciphers);
    vector<Ciphertext> selection_vector(q_params->db_->num_keywords);

    // Loading Data
    for (int i=0;i<q_params->num_input_ciphers;i++){
        encrypted_query[i].load(*context, data_stream);
    }

    if (q_params->num_threads == 0){
        q_params->num_threads = omp_get_max_threads();
    } else {
        omp_set_num_threads(q_params->num_threads);
    }
    if (_verbose) cout << "\tNumber of Threads: " << q_params->num_threads << endl
                        << "--------------------------------------------------------------------" << endl;


    // Expansion
    vector<Ciphertext> expanded_query;
    this->time_start = chrono::high_resolution_clock::now();
        expanded_query = expand_input_ciphers(encrypted_query, q_params->num_input_ciphers, q_params->encoding_size);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    q_params->metrics_["time_expansion"] = this->time_diff.count();

    // Generate selection vector
    this->time_start = chrono::high_resolution_clock::now();
        generate_selection_vector(q_params, expanded_query, selection_vector, _verbose);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    q_params->metrics_["time_sel_vec"] = this->time_diff.count();

    // Inner product
    this->time_start = chrono::high_resolution_clock::now();
        faster_inner_product(q_params, selection_vector, encrypted_answer, _verbose);
    this->time_end = chrono::high_resolution_clock::now();
    this->time_diff = chrono::duration_cast<chrono::microseconds>(this->time_end - this->time_start);
    q_params->metrics_["time_inner_prod"] = this->time_diff.count();

    for (uint64_t o = 0; o < q_params->num_output_ciphers; o++){
        this->evaluator->transform_from_ntt_inplace(encrypted_answer[o]);
    }

    for (uint64_t o = 0; o < q_params->num_output_ciphers; o++){
        this->evaluator->mod_switch_to_inplace(encrypted_answer[o], context->last_context_data()->parms_id());
    }

    if (_verbose) cout << "--- End of process ---" << endl;
    // if (DEBUG && _verbose) {
    //     cout << "Final Noise Level: " << noise_calculator->invariant_noise_budget(encrypted_answer[0]) << endl;
    // }

    data_stream.str("");

    auto size_encrypted_answer = 0;

    for (uint64_t o = 0; o < q_params->num_output_ciphers; o++){
        size_encrypted_answer += encrypted_answer[o].save(data_stream);
    }
    time_end_local = chrono::high_resolution_clock::now();
    time_diff_local = chrono::duration_cast<chrono::microseconds>(time_end_local - time_start_local);
    q_params->metrics_["time_server_total"] = time_diff_local.count();

    // // Printing timings
    // if (_verbose) cout << "\tDatabase Prep   : " << setw(10) << q_params->metrics_["time_prep"]/1000 << " ms" << endl;
    // if (_verbose) cout << "\tExpansion time: : " << setw(10) << q_params->metrics_["time_expansion"]/1000 << " ms" << endl;
    // if (_verbose) cout << "\tSel. Vec. Calc. : " << setw(10) << q_params->metrics_["time_sel_vec"]/1000 << " ms" << endl;
    // if (_verbose) cout << "\tInner Product   : " << setw(10) << q_params->metrics_["time_inner_prod"]/1000 << " ms" << endl;
    // if (_verbose) cout << "\tTotal Server    : " << setw(10) << q_params->metrics_["time_server_total"]/1000 << " ms" << endl;

}
