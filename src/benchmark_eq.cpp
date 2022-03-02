#include "utils.h"
#include <iostream>
#include <chrono>
#include <unistd.h>  
#include <stdio.h>  

#include "seal/seal.h"

using namespace seal;
using namespace std;

int binary_folklore(uint64_t ell, uint64_t log_poly_modulus_degree, bool batched=false, bool parallel = false, string write_path="./"){
    
    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    
    uint64_t plaintext_modulus_bitsize = 20;

    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        params.set_plain_modulus(1 << 20);
    } 

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    vector<Ciphertext> cts_x_bits_;
    vector<Ciphertext> cts_y_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<ell;i++){
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            cts_y_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<ell;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            cts_y_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    // 1 + x[i] + y[i]
    vector<Ciphertext> cts_bits_;
    for (int i=0;i<ell;i++){
        Ciphertext _ct;
        evaluator.add(cts_x_bits_[i], cts_y_bits_[i], _ct);
        if (batched){
            vector<uint64_t> raw_data(poly_modulus_degree, 1);
            batch_encoder->encode(raw_data, pt);
            evaluator.add_plain_inplace(_ct, pt);
        } else {
            evaluator.add_plain_inplace(_ct, Plaintext("1"));
        }
        cts_bits_.push_back(_ct);
    }

    // Multiply
    Ciphertext _ct_result;
    if (!parallel){
        evaluator.multiply_many(cts_bits_, rlk, _ct_result);
    } else {
        uint64_t ceil_log_ell = ceil(log2(ell));
        for (int i=0;i<ceil_log_ell;i++){
            #pragma omp parallel
            {
                #pragma omp for
                for (int j=0;j<1<<(ceil_log_ell-1-i);j++){
                    if (j + (1<<(ceil_log_ell-1-i)) < cts_bits_.size()){
                        evaluator.multiply_inplace(cts_bits_[j], cts_bits_[j + (1<<(ceil_log_ell-1-i))]);
                        evaluator.relinearize_inplace(cts_bits_[j], rlk);
                    }
                }
            }
        }
        _ct_result = cts_bits_[0];
    }
    
    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    Plaintext _pt_result;
    dec.decrypt(_ct_result, _pt_result);
    if (dec.invariant_noise_budget(_ct_result) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,ell,poly_mod_degree,batched,num_threads,runtime" << endl;
    // output_file << _id << ",fl-bin," << ell << "," << poly_modulus_degree << "," << batched << "," << num_threads << "," << runtime_;

    output_file << "id," << _id << endl <<
                   "type," << "fl-bin" << endl <<
                   "ell," << ell << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;

    output_file.close();

    return runtime_;

}

int arithmetic_folklore(uint64_t ell, uint64_t log_poly_modulus_degree, bool batched=false, bool parallel = false, string write_path="./"){
    
    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
        Modulus mod(_prime);
        params.set_plain_modulus(mod);
    }

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    vector<Ciphertext> cts_x_bits_;
    vector<Ciphertext> cts_y_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<ell;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<ell;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    // 1 - (x[i] - y[i])^2
    vector<Ciphertext> cts_bits_;
    for (int i=0;i<ell;i++){
        Ciphertext _ct;
        evaluator.sub(cts_x_bits_[i], cts_y_bits_[i], _ct);
        evaluator.square_inplace(_ct);
        evaluator.relinearize_inplace(_ct, rlk);
        if (batched){
            vector<uint64_t> raw_data(poly_modulus_degree, 1);
            batch_encoder->encode(raw_data, pt);
            evaluator.sub_plain_inplace(_ct, pt);
            evaluator.negate_inplace(_ct);
        } else {
            evaluator.sub_plain_inplace(_ct, Plaintext("1"));
            evaluator.negate_inplace(_ct);
        }
        cts_bits_.push_back(_ct);
    }

    // Multiply
    Ciphertext _ct_result;
    if (!parallel){
        evaluator.multiply_many(cts_bits_, rlk, _ct_result);
    } else {
        uint64_t ceil_log_ell = ceil(log2(ell));
        for (int i=0;i<ceil_log_ell;i++){
            #pragma omp parallel
            {
                #pragma omp for
                for (int j=0;j<1<<(ceil_log_ell-1-i);j++){
                    if (j + (1<<(ceil_log_ell-1-i)) < cts_bits_.size()){
                        evaluator.multiply_inplace(cts_bits_[j], cts_bits_[j + (1<<(ceil_log_ell-1-i))]);
                        evaluator.relinearize_inplace(cts_bits_[j], rlk);
                    }
                }
            }
        }
        _ct_result = cts_bits_[0];
    }
    
    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    Plaintext _pt_result;
    dec.decrypt(_ct_result, _pt_result);
    if (dec.invariant_noise_budget(_ct_result) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,ell,poly_mod_degree,batched,num_threads,runtime" << endl;
    // output_file << _id << ",fl-arith," << ell << "," << poly_modulus_degree << "," << batched << "," << num_threads << "," << runtime_;

    output_file << "id," << _id << endl <<
                   "type," << "fl-arith" << endl <<
                   "ell," << ell << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;

    output_file.close();

    return runtime_;

}

int plain_folklore(uint64_t ell, uint64_t log_poly_modulus_degree, bool batched=false, bool parallel = false, string write_path="./"){
    
    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
        Modulus mod(_prime);
        params.set_plain_modulus(mod);
    }

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    vector<Ciphertext> cts_x_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<ell;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<ell;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    // x[i] or (1 - x[i])
    vector<Ciphertext> cts_bits_;
    for (int i=0;i<ell;i++){
        if (i%2==0) {
            if (batched){
                vector<uint64_t> raw_data(poly_modulus_degree, 1);
                batch_encoder->encode(raw_data, pt);
                evaluator.sub_plain_inplace(cts_x_bits_[i], pt);
                evaluator.negate_inplace(cts_x_bits_[i]);
            } else {
                evaluator.sub_plain_inplace(cts_x_bits_[i], Plaintext("1"));
                evaluator.negate_inplace(cts_x_bits_[i]);
            }
        }
        cts_bits_.push_back(cts_x_bits_[i]);
    }

    // Multiply
    Ciphertext _ct_result;
    if (!parallel){
        evaluator.multiply_many(cts_bits_, rlk, _ct_result);
    } else {
        uint64_t ceil_log_ell = ceil(log2(ell));
        for (int i=0;i<ceil_log_ell;i++){
            #pragma omp parallel
            {
                #pragma omp for
                for (int j=0;j<1<<(ceil_log_ell-1-i);j++){
                    if (j + (1<<(ceil_log_ell-1-i)) < cts_bits_.size()){
                        evaluator.multiply_inplace(cts_bits_[j], cts_bits_[j + (1<<(ceil_log_ell-1-i))]);
                        evaluator.relinearize_inplace(cts_bits_[j], rlk);
                    }
                }
            }
        }
        _ct_result = cts_bits_[0];
    }
    
    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    Plaintext _pt_result;
    dec.decrypt(_ct_result, _pt_result);
    if (dec.invariant_noise_budget(_ct_result) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,ell,poly_mod_degree,batched,num_threads,runtime" << endl;
    // output_file << _id << ",fl-plain," << ell << "," << poly_modulus_degree << "," << batched << "," << num_threads << "," << runtime_;

    output_file << "id," << _id << endl <<
                   "type," << "fl-plain" << endl <<
                   "ell," << ell << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;

    output_file.close();

    return runtime_;

}

int binary_raffle(uint64_t bitlength, uint64_t alpha, uint64_t log_poly_modulus_degree, bool batched=false, bool parallel = false, string write_path="./"){

    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        params.set_plain_modulus(1<<20);
    }

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    vector<Ciphertext> cts_x_bits_;
    vector<Ciphertext> cts_y_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<bitlength;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<bitlength;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    srand(time(NULL));
    vector<Ciphertext> cts_;
    uint64_t k = alpha;
    for (int i=0;i<k;i++){
        vector<Ciphertext> ct_sum_ops_;
        for (int i=0;i<bitlength;i++){
            if (rand() % 2 == 1){
                ct_sum_ops_.push_back(cts_x_bits_[i]);
                ct_sum_ops_.push_back(cts_y_bits_[i]);
            }
        }
        evaluator.add_many(ct_sum_ops_, _ct);
        if (batched){
            vector<uint64_t> raw_data(poly_modulus_degree, 1);
            batch_encoder->encode(raw_data, pt);
            evaluator.add_plain_inplace(_ct, pt);
        } else {
            evaluator.add_plain_inplace(_ct, Plaintext("1"));
        }
        cts_.push_back(_ct);
    }

    Ciphertext _ct_all;
    if (!parallel){
        evaluator.multiply_many(cts_, rlk, _ct_all);
    } else {
        uint64_t ceil_log_alpha = ceil(log2(alpha));
        for (int i=0;i<ceil_log_alpha;i++){
            #pragma omp parallel
            {
                #pragma omp for
                for (int j=0;j<1<<(ceil_log_alpha-1-i);j++){
                    if (j + (1<<(ceil_log_alpha-1-i)) < cts_.size()){
                        evaluator.multiply_inplace(cts_[j], cts_[j + (1<<(ceil_log_alpha-1-i))]);
                        evaluator.relinearize_inplace(cts_[j], rlk);
                    }
                }
            }
        }
        _ct_all = cts_[0];
    }

    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    Plaintext _pt_all;
    dec.decrypt(_ct_all, _pt_all);
    if (dec.invariant_noise_budget(_ct_all) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,bitlength,alpha,hamming_weight,poly_mod_degree,batched,num_threads,runtime" << endl;
    // output_file << _id << ",br," << bitlength << "," << alpha << "," << 0 << "," << poly_modulus_degree << "," << batched << "," << num_threads << "," << runtime_;

    output_file << "id," << _id << endl <<
                   "type," << "br" << endl <<
                   "bitlength," << bitlength << endl <<
                   "alpha," << alpha << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;

    output_file.close();

    return runtime_;

}

int constant_weight_binary(uint64_t m, uint64_t k, uint64_t log_poly_modulus_degree, uint64_t alpha_or_ell=0, bool batched=false, bool parallel = false, string write_path="./"){

    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        params.set_plain_modulus(1<<20);
    }

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    uint64_t log_k = ceil(log2(k));

    if (alpha_or_ell != 0){
        m = find_log2_choose(k, alpha_or_ell);
    }

    vector<Ciphertext> cts_x_bits_;
    vector<Ciphertext> cts_y_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<m;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<m;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    }
    
    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    if (parallel){
        #pragma omp parallel
        {
            #pragma omp for
            for (int i=0;i<m;i++){
                evaluator.multiply_inplace(cts_x_bits_[i], cts_y_bits_[i]);
                evaluator.relinearize_inplace(cts_x_bits_[i], rlk);
            }
        }
    } else {
        for (int i=0;i<m;i++){
            evaluator.multiply_inplace(cts_x_bits_[i], cts_y_bits_[i]);
            evaluator.relinearize_inplace(cts_x_bits_[i], rlk);
        }
    }
    
    vector<Ciphertext> cts_b;
    evaluator.add_many(cts_x_bits_, _ct);
    cts_b.push_back(_ct);

    vector<Ciphertext> cts_S(m, Ciphertext());

    for (int i=0;i<log_k;i++){

        if (i%2==0) {
            enc.encrypt(Plaintext(to_string(0)), cts_S[0]);
        } else {
            enc.encrypt(Plaintext(to_string(0)), cts_S[m-1]);
        }
        
        for (int j=1;j<m;j++){
            Ciphertext _ct;
            if (i%2==0) evaluator.add(cts_S[j-1], cts_x_bits_[j-1], cts_S[j]);
            else evaluator.add(cts_S[m-j], cts_x_bits_[m-j], cts_S[m-1-j]);
        }

        if (parallel){
            #pragma omp parallel
            {
                #pragma omp for
                for (int j=0;j<m;j++){
                    evaluator.multiply_inplace(cts_x_bits_[j], cts_S[j]);
                    evaluator.relinearize_inplace(cts_x_bits_[j], rlk);
                }
            }
        } else {
            for (int j=0;j<m;j++){
                evaluator.multiply_inplace(cts_x_bits_[j], cts_S[j]);
                evaluator.relinearize_inplace(cts_x_bits_[j], rlk);
            }
        }
        evaluator.add_many(cts_x_bits_, _ct);
        cts_b.push_back(_ct);
    }
    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    dec.decrypt(cts_b[log_k], pt);

    if (dec.invariant_noise_budget(cts_b[log_k]) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,encoding_size,hamming_weight,poly_mod_degree,alpha_or_ell,batched,num_threads,runtime" << endl;
    // output_file << _id << ",cw-bin," << m << "," << k << "," << poly_modulus_degree << "," << alpha_or_ell << "," << batched << "," << num_threads <<  "," << runtime_;
    
    output_file << "id," << _id << endl <<
                   "type," << "cw-bin" << endl <<
                   "encoding_size," << m << endl <<
                   "hamming_weight," << k << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "alpha_or_ell," << alpha_or_ell << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;
    
    output_file.close();

    return runtime_;
}

int constant_weight_arith(uint64_t m, uint64_t k, uint64_t log_poly_modulus_degree, uint64_t alpha_or_ell=0, bool batched=false, bool parallel = false, string write_path="./"){

    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
        Modulus mod(_prime);
        params.set_plain_modulus(mod);
    } 

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    // uint64_t k = 1 << log_k;
    uint64_t log_k = ceil(log2(k));

    if (alpha_or_ell != 0){
        m = find_log2_choose(k, alpha_or_ell);
    }

    vector<Ciphertext> cts_x_bits_;
    vector<Ciphertext> cts_y_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<m;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<m;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
            enc.encrypt(pt, _ct);
            cts_y_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    if (parallel){
        #pragma omp parallel
        {
            #pragma omp for
            for (int i=0;i<m;i++){
                evaluator.multiply_inplace(cts_x_bits_[i], cts_y_bits_[i]);
                evaluator.relinearize_inplace(cts_x_bits_[i], rlk);
            }
        }
    } else {
        for (int i=0;i<m;i++){
            evaluator.multiply_inplace(cts_x_bits_[i], cts_y_bits_[i]);
            evaluator.relinearize_inplace(cts_x_bits_[i], rlk);
        }
    }
    
    evaluator.add_many(cts_x_bits_, _ct);
    vector<Ciphertext> cts_ops;
    for (int i=1;i<k;i++){
        Ciphertext ct_temp;
        evaluator.sub_plain(_ct, Plaintext(to_string(i)), ct_temp);
        cts_ops.push_back(ct_temp);
    }
    cts_ops.push_back(_ct);

    if (k > 1) {
        if (!parallel){
            evaluator.multiply_many(cts_ops, rlk, _ct);
        } else {
            uint64_t ceil_log_k = ceil(log2(k));
            for (int i=0;i<ceil_log_k;i++){
                #pragma omp parallel
                {
                    #pragma omp for
                    for (int j=0;j<1<<(ceil_log_k-1-i);j++){
                        if (j + (1<<(ceil_log_k-1-i)) < cts_ops.size()){
                            evaluator.multiply_inplace(cts_ops[j], cts_ops[j + (1<<(ceil_log_k-1-i))]);
                            evaluator.relinearize_inplace(cts_ops[j], rlk);
                        }
                    }
                }
            }
            _ct = cts_ops[0];
        }
        if (batched){
            vector<uint64_t> raw_data(poly_modulus_degree, 12132);
            batch_encoder->encode(raw_data, pt);
            evaluator.multiply_plain_inplace(_ct, pt);
        } else {
            evaluator.multiply_plain_inplace(_ct, Plaintext("12132"));
        }
    } else {
        _ct = cts_ops[0];
    }

    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    dec.decrypt(_ct, pt);

    if (dec.invariant_noise_budget(_ct) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,encoding_size,hamming_weight,poly_mod_degree,alpha_or_ell,batched,num_threads,runtime" << endl;
    // output_file << _id << ",cw-arith," << m << "," << k << "," << poly_modulus_degree << "," << alpha_or_ell << "," << batched << "," << num_threads << "," << runtime_;

    output_file << "id," << _id << endl <<
                   "type," << "cw-arith" << endl <<
                   "encoding_size," << m << endl <<
                   "hamming_weight," << k << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "alpha_or_ell," << alpha_or_ell << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;
    
    output_file.close();

    return runtime_;
}

int constant_weight_plain(uint64_t m, uint64_t k, uint64_t log_poly_modulus_degree, uint64_t alpha_or_ell=0, bool batched=false, bool parallel = false, string write_path="./"){

    uint64_t poly_modulus_degree = 1 << log_poly_modulus_degree;
    uint64_t num_threads = ( parallel ? omp_get_max_threads() : 0 );

    chrono::high_resolution_clock::time_point time_;

    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    uint64_t plaintext_modulus_bitsize = 20;
    if (batched){
        params.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, plaintext_modulus_bitsize));
    } else {
        uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
        Modulus mod(_prime);
        params.set_plain_modulus(mod);
    } 

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);
    
    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct;
    Plaintext pt;

    // uint64_t k = 1 << log_k;
    uint64_t log_k = ceil(log2(k));

    if (alpha_or_ell != 0){
        m = find_log2_choose(k, alpha_or_ell);
    }

    vector<Ciphertext> cts_x_bits_;
    BatchEncoder* batch_encoder;
    if (batched) {
        batch_encoder = new BatchEncoder(context);
        vector<uint64_t> raw_data(poly_modulus_degree, 1);
        for (int i=0;i<m;i++){
            Plaintext pt;
            batch_encoder->encode(raw_data, pt);
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
        }
    } else {
        for (int i=0;i<m;i++){
            pt = Plaintext("1");
            enc.encrypt(pt, _ct);
            cts_x_bits_.push_back(_ct);
        }
    }

    // ---------------- Start Timing ----------------- //
    time_ = chrono::high_resolution_clock::now();

    vector<Ciphertext> cts_ops;
    for (int i=0;i<k;i++){
        cts_ops.push_back(cts_x_bits_[i]);
    }

    if (k > 1){
        if (!parallel){
            evaluator.multiply_many(cts_ops, rlk, _ct);
        } else {
            uint64_t ceil_log_k = ceil(log2(k));
            for (int i=0;i<ceil_log_k;i++){
                #pragma omp parallel
                {
                    #pragma omp for
                    for (int j=0;j<1<<(ceil_log_k-1-i);j++){
                        if (j + (1<<(ceil_log_k-1-i)) < cts_ops.size()){
                            evaluator.multiply_inplace(cts_ops[j], cts_ops[j + (1<<(ceil_log_k-1-i))]);
                            evaluator.relinearize_inplace(cts_ops[j], rlk);
                        }
                    }
                }
            }
            _ct = cts_ops[0];
        }
    } else {
        _ct = cts_ops[0];
    }

    int64_t runtime_ = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count();
    // ---------------- End Timing ----------------- //

    dec.decrypt(_ct, pt);

    if (dec.invariant_noise_budget(_ct) == 0){
        runtime_ = -1;
    }

    ofstream output_file;
    uint64_t _id = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000;
    string filename=write_path + to_string(_id);
    output_file.open( filename + ".csv");
    // output_file << "id,type,encoding_size,hamming_weight,poly_mod_degree,alpha_or_ell,batched,num_threads,runtime" << endl;
    // output_file << _id << ",cw-plain," << m << "," << k << "," << poly_modulus_degree << "," << alpha_or_ell << "," << batched << "," << num_threads << "," << runtime_;
    output_file << "id," << _id << endl <<
                   "type," << "cw-plain" << endl <<
                   "encoding_size," << m << endl <<
                   "hamming_weight," << k << endl <<
                   "poly_mod_degree," << poly_modulus_degree << endl <<
                   "alpha_or_ell," << alpha_or_ell << endl <<
                   "batched," << batched << endl <<
                   "num_threads," << num_threads << endl <<
                   "runtime," << runtime_ << endl;
    
    output_file.close();

    return runtime_;
}


void benchmark_ops(uint64_t log_poly_modulus_degree=13, int RUNS=1000){

    cout << " ------------- Benchmarks for N=" << (1 << log_poly_modulus_degree) << " ------------- " << endl;

    chrono::high_resolution_clock::time_point time_;

    uint64_t poly_modulus_degree=1<<log_poly_modulus_degree;
    EncryptionParameters params(scheme_type::bfv);
    params.set_poly_modulus_degree(poly_modulus_degree);
    params.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    
    // uint64_t _prime = (1<<20)+(1<<19)+(1<<17)+(1<<16)+(1<<14)+1;
    uint64_t _prime = PlainModulus::Batching(poly_modulus_degree, 20).value();
    params.set_plain_modulus(_prime);

    auto context = SEALContext(params);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);

    Evaluator evaluator(context);
    RelinKeys rlk;
    if(log_poly_modulus_degree > 11) keygen.create_relin_keys(rlk);

    GaloisKeys gal_keys;
    if(log_poly_modulus_degree > 11) keygen.create_galois_keys(gal_keys);

    Encryptor enc(context, pk);
    Decryptor dec(context, sk);

    Ciphertext _ct_1, _ct_2, _ct_3;
    Plaintext pt, pt_small, pt_large;

    pt = Plaintext(vector_to_poly_string(vector<uint64_t>(poly_modulus_degree, _prime-1)));
    pt_small = Plaintext("2");
    pt_large = Plaintext(vector_to_poly_string(vector<uint64_t>(poly_modulus_degree, _prime-1)));
    enc.encrypt(pt, _ct_1);
    enc.encrypt(pt, _ct_2);

    cout << "Initial Noise: " << dec.invariant_noise_budget(_ct_1) << endl;

    time_ = chrono::high_resolution_clock::now();
    for (int i=0;i<RUNS;i++){
        evaluator.add(_ct_1, _ct_2, _ct_3);
    }
    cout << "Average Add Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
    // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;

    time_ = chrono::high_resolution_clock::now();
    for (int i=0;i<RUNS;i++){
        evaluator.multiply_plain(_ct_1, pt_small, _ct_3);
    }
    cout << "Average (Small) Plain Mult Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
    // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;
    // cout << dec.invariant_noise_budget(_ct_3) << endl;

    time_ = chrono::high_resolution_clock::now();
    for (int i=0;i<RUNS;i++){
        evaluator.multiply_plain(_ct_1, pt_large, _ct_3);
    }
    cout << "Average (Large) Plain Mult Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
    // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;
    // cout << dec.invariant_noise_budget(_ct_3) << endl;

    if(log_poly_modulus_degree > 11) {
        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.multiply(_ct_1, _ct_2, _ct_3);
        }
        cout << "Average Mult Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
        // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;
        // cout << dec.invariant_noise_budget(_ct_3) << endl;
        //     evaluator.multiply_plain(_ct_3, pt, _ct_3);
        // cout << dec.invariant_noise_budget(_ct_3) << endl;


        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.relinearize_inplace(_ct_3, rlk);
        }
        cout << "Average Relin Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
        // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;

        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.apply_galois(_ct_1, 3, gal_keys, _ct_3);
        }
        cout << "Average Substitution Time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
        // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;

        Ciphertext transform_ct_1, transform_ct_2;
        Plaintext transform_pt;
        
        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.transform_to_ntt(pt, context.first_parms_id(), transform_pt);
        }
        cout << "Average Transform Plain to NTT time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;

        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.transform_to_ntt(_ct_1, transform_ct_1);
        }
        cout << "Average Transform to NTT time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;

        evaluator.transform_to_ntt(_ct_1, transform_ct_1);
        evaluator.transform_to_ntt(_ct_2, transform_ct_2);
        evaluator.transform_to_ntt(pt, context.first_parms_id(), transform_pt);
        
        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.multiply_plain(transform_ct_1, transform_pt, _ct_3);
        }
        cout << "Average Plain Mult Time (With NTT): " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;

        evaluator.transform_to_ntt(_ct_1, transform_ct_1);
        evaluator.transform_to_ntt(_ct_2, transform_ct_2);
        evaluator.transform_to_ntt(pt, context.first_parms_id(), transform_pt);

        // time_ = chrono::high_resolution_clock::now();
        // for (int i=0;i<RUNS;i++){
        //     evaluator.multiply(transform_ct_1, transform_ct_2, _ct_3);
        // }
        // cout << "Average Mult Time (With NTT): " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;

        evaluator.transform_to_ntt(_ct_1, transform_ct_1);
        evaluator.transform_to_ntt(_ct_2, transform_ct_2);
        evaluator.transform_to_ntt(pt, context.first_parms_id(), transform_pt);

        time_ = chrono::high_resolution_clock::now();
        for (int i=0;i<RUNS;i++){
            evaluator.transform_from_ntt(transform_ct_1, _ct_3);
        }
        cout << "Average Transform from NTT time: " << chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - time_).count() / RUNS << endl;
        // cout << ((dec.invariant_noise_budget(_ct_3) > 0) ? " Good" : "Bad") << endl;
    
    }
}

uint64_t code_size_from_ell(uint64_t ell, uint64_t k){
    return find_log2_choose(k, ell);
}

int main(int argc, char *argv[]){ 
    uint64_t hamming_weight=2, log_poly_mod_degree=13, code_size=0, alpha = 0, bitlength, ell=0;
    bool batched=false, parallel=false;

    string write_path = "";

    if (argc < 2){
        cout << "Try again" << endl;
        return 0;
    }

    const char * type = argv[1];

    int opt;
    while((opt = getopt(argc, argv, "k:d:w:a:m:b:l:vp")) != -1){  
        switch(opt){
            case 'm':
                code_size = stoi(optarg);
                break;
            case 'k':
                hamming_weight = stoi(optarg);
                break;
            case 'd':
                log_poly_mod_degree = stoi(optarg);
                break;
            case 'a':
                alpha = stoi(optarg);
                break;
            case 'b':
                bitlength = stoi(optarg);
                break;
            case 'l':
                ell = stoi(optarg);
                break;
            case 'w':
                write_path = optarg;
                break;
            case 'v':
                batched = true;
                break;
            case 'p':
                parallel = true;
                break;
            case ':':
                printf("option needs a value\n");  
                break;
            case '?':
                printf("unknown option: %c\n", optopt); 
                break; 
        }
    }
    
    if (strcmp(type,"cw-bin")==0){
        std::cout << "Constant-weight Binary" << endl;
        if (code_size == 0) code_size = code_size_from_ell(ell, hamming_weight);
        constant_weight_binary(code_size, hamming_weight, log_poly_mod_degree, max(alpha,ell), batched, parallel, write_path);
    } else if (strcmp(type,"cw-arith")==0){
        std::cout << "Constant-weight Arithmetic" << endl;
        if (code_size == 0) code_size = code_size_from_ell(ell, hamming_weight);
        constant_weight_arith(code_size, hamming_weight, log_poly_mod_degree, max(alpha,ell) , batched, parallel, write_path);
    } else if (strcmp(type,"cw-plain")==0){
        std::cout << "Constant-weight Plain" << endl;
        if (code_size == 0) code_size = code_size_from_ell(ell, hamming_weight);
        constant_weight_plain(code_size, hamming_weight, log_poly_mod_degree, max(alpha,ell) , batched, parallel, write_path);
    } else if (strcmp(type,"br")==0) {
        std::cout << "Binary Raffle" << endl;
        binary_raffle(bitlength, alpha, log_poly_mod_degree, batched, parallel, write_path);
    } else if (strcmp(type,"fl-bin")==0) {
        std::cout << "Binary Folklore" << endl;
        binary_folklore(ell, log_poly_mod_degree, batched, parallel, write_path);
    } else if (strcmp(type,"fl-arith")==0) {
        std::cout << "Arithmetic Folklore" << endl;
        arithmetic_folklore(ell, log_poly_mod_degree, batched, parallel, write_path);
    } else if (strcmp(type,"fl-plain")==0) {
        std::cout << "Plain Folklore" << endl;
        plain_folklore(ell, log_poly_mod_degree, batched, parallel, write_path);
    }

}