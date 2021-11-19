#include <iostream>
#include <gtest/gtest.h>

#include "../src/utils.h"
#include "../src/database.h"
#include "../src/PIRserver.h"
#include "../src/client.h"


chrono::high_resolution_clock::time_point time_start, time_end;
chrono::microseconds time_diff;


pair<string, pair<uint64_t, uint64_t>> get_triplet(string x, uint64_t y, uint64_t z){
    return pair<string, pair<uint64_t, uint64_t>>(x , pair<uint64_t, uint64_t>(y, z));
}

TEST(POLY_STRING_TEST, monomial){
    pair<uint64_t, uint64_t> ans;

    pair<string, pair<uint64_t, uint64_t>> test_cases[] = {
        get_triplet(" Ab ", 0, 171),
        get_triplet(" acx ", 1, 172),
        get_triplet(" a1x^12 ", 12, 161),
    };

    for (pair<string, pair<uint64_t, uint64_t>> test_case : test_cases){
        ans = monomial_to_integer(test_case.first);
        
        ASSERT_EQ(ans.first,test_case.second.first);
        ASSERT_EQ(ans.second,test_case.second.second);
    }

}

TEST(POLY_STRING_TEST, polynomial){

    vector<uint64_t> ans2, true_coefficients({3,171,1,0,0,0,0,0,0,0});
    ans2 = poly_string_to_vector("1x^2 + abx + 3", 10);
    for (int i=0;i<true_coefficients.size();i++){
        ASSERT_EQ(ans2[i], true_coefficients[i]);
    }

    vector<uint64_t> coeffs({1,6,11,16,21});
    ASSERT_EQ(vector_to_poly_string(coeffs).compare("15x^4 + 10x^3 + bx^2 + 6x^1 + 1"), 0);

    for (int j=5;j<100;j++){
        coeffs.push_back(j*5+1);
    }
    ASSERT_EQ(vector_to_poly_string(coeffs).compare(
        "1f0x^99 + 1ebx^98 + 1e6x^97 + 1e1x^96 + 1dcx^95 + 1d7x^94 + 1d2x^93 + 1cdx^92 + 1c8x^91 + 1c3x^90 + 1bex^89 + 1b9x^88 + 1b4x^87 + 1afx^86 + 1aax^85 + 1a5x^84 + 1a0x^83 + 19bx^82 + 196x^81 + 191x^80 + 18cx^79 + 187x^78 + 182x^77 + 17dx^76 + 178x^75 + 173x^74 + 16ex^73 + 169x^72 + 164x^71 + 15fx^70 + 15ax^69 + 155x^68 + 150x^67 + 14bx^66 + 146x^65 + 141x^64 + 13cx^63 + 137x^62 + 132x^61 + 12dx^60 + 128x^59 + 123x^58 + 11ex^57 + 119x^56 + 114x^55 + 10fx^54 + 10ax^53 + 105x^52 + 100x^51 + fbx^50 + f6x^49 + f1x^48 + ecx^47 + e7x^46 + e2x^45 + ddx^44 + d8x^43 + d3x^42 + cex^41 + c9x^40 + c4x^39 + bfx^38 + bax^37 + b5x^36 + b0x^35 + abx^34 + a6x^33 + a1x^32 + 9cx^31 + 97x^30 + 92x^29 + 8dx^28 + 88x^27 + 83x^26 + 7ex^25 + 79x^24 + 74x^23 + 6fx^22 + 6ax^21 + 65x^20 + 60x^19 + 5bx^18 + 56x^17 + 51x^16 + 4cx^15 + 47x^14 + 42x^13 + 3dx^12 + 38x^11 + 33x^10 + 2ex^9 + 29x^8 + 24x^7 + 1fx^6 + 1ax^5 + 15x^4 + 10x^3 + bx^2 + 6x^1 + 1"
        ),0);
}

TEST(ENCODING_TEST, get_codeword){

    vector<vector<uint64_t>> inputs={ //number, code length, hamming_weight
        {10231,100,4}
    };
    vector<vector<uint64_t>> outputs={
        {0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    bool __pass;
    for (int i=0;i<inputs.size();i++){

        vector<uint64_t> in = inputs[i];
        vector<uint64_t> out = outputs[i];
        
        vector<uint64_t> output2;
        output2 = get_perfect_constant_weight_codeword(in[0], in[1], in[2]);

        for (int j=0;j<output2.size();j++){
            ASSERT_EQ(output2[j], out[j]);
        }

    }
}

TEST(ROTATION, rotation){

    PIRserver s;
    s.initialize();
    vector<uint64_t> coeffs;
    for (int j=1;j<10;j++) coeffs.push_back(j);
    // Plaintext pt("1x^34 + 1x^12");
    Plaintext pt(vector_to_poly_string(coeffs));

    Ciphertext ct, ct2;
    s.enc->encrypt(pt, ct);
    
    s.multiply_inverse_power_of_x(ct, 2, ct2);

    s.noise_calculator->decrypt(ct2, pt);

    ASSERT_EQ(pt.to_string().compare("1B3FFFx^8191 + 1B4000x^8190 + 9x^6 + 8x^5 + 7x^4 + 6x^3 + 5x^2 + 4x^1 + 3"),0);

}

TEST(EXPANSION, expansion){

    PIRserver s;
    s.initialize();

    // Plaintext pt("1x^7 + 1x^5 + 1x^4 + 1x^1 + 1");
    Plaintext pt("1x^3 + 1x^2 + 1");
    uint64_t coeffs[] = {1,0,1,1};
    Ciphertext ct;
    s.enc->encrypt(pt, ct);
    int num_bits = 3;
    vector<Ciphertext> cts = s.expand_procedure(ct, num_bits);

    for (int i=0;i<num_bits;i++){
        try {
            s.noise_calculator->decrypt(cts[i], pt);
            ASSERT_EQ(stoi(pt.to_string()),4*coeffs[i]);
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << '\n';
        }
        
    }
}

class PIRTest : public ::testing::Test {
    protected:
    virtual void SetUp();
    void initialize_db_params(uint64_t, uint64_t);
    void initialize_query_params(uint64_t);
    vector<Ciphertext> binary_vec_to_encrypted_query(vector<uint64_t>);
    uint64_t hamming_weight;
    uint64_t num_keywords;
    uint64_t log_poly_mod_degree;
    uint64_t poly_mod_degree;
    uint64_t domain_bitlength;
    uint64_t response_size_in_bytes;

    Database* db;
    PIRserver* s;
    QueryParameters* query_parameters;

    Ciphertext *ct_one, *ct_zero;

};

void PIRTest::SetUp(){
    this->log_poly_mod_degree=13;
    this->poly_mod_degree=1<<log_poly_mod_degree;


    s = new PIRserver();
    s->initialize(poly_mod_degree);    

    Plaintext pt_one("1"), pt_zero("0");
    ct_one = new Ciphertext;
    ct_zero = new Ciphertext;
    s->enc->encrypt(pt_one, *ct_one);
    s->enc->encrypt(pt_zero, *ct_zero);


}

void PIRTest::initialize_db_params(uint64_t _num_keywords=100, uint64_t _domain_bitlength=12){
    this->num_keywords=_num_keywords;
    this->domain_bitlength = _domain_bitlength;//(uint64_t)(ceil(log2(num_keywords)));
    this->response_size_in_bytes = poly_mod_degree*5/2;
    this->db = new Database(num_keywords, response_size_in_bytes, domain_bitlength);
}

void PIRTest::initialize_query_params(uint64_t _hamming_weight){
    this->hamming_weight=_hamming_weight;
    query_parameters = new QueryParameters(log_poly_mod_degree, Constant_Weight, hamming_weight, db, domain_bitlength);
}

vector<Ciphertext> PIRTest::binary_vec_to_encrypted_query(vector<uint64_t> binary_vec){
    vector<Ciphertext> encrypted_query;
    for (int i=0;i<binary_vec.size();i++){
        encrypted_query.push_back((binary_vec[i] == 0) ? *ct_zero : *ct_one);
    }
    return encrypted_query;
}

TEST_F(PIRTest, generate_selection_bit){

    this->initialize_db_params(100, 12);
    this->initialize_query_params(4);
    vector<Ciphertext> encrypted_query = binary_vec_to_encrypted_query(query_parameters->get_encoding_of_(1));

    Plaintext pt;
    Ciphertext res = s->generate_selection_bit(query_parameters, 1, encrypted_query);
    s->noise_calculator->decrypt(res, pt);
    ASSERT_EQ(stoi(pt.to_string()),1);
}

TEST_F(PIRTest, generate_selection_vector){
    this->initialize_db_params(100, 12);
    this->initialize_query_params(4);
    
    vector<Ciphertext> selection_vector(query_parameters->db_->num_keywords);

    uint64_t keyword_index = 1;
    uint64_t keyword = query_parameters->db_->keywords[keyword_index];

    vector<Ciphertext> encrypted_query = binary_vec_to_encrypted_query(query_parameters->get_encoding_of_(keyword));

    s->generate_selection_vector(query_parameters, encrypted_query, selection_vector, false);

    Plaintext pt;
    for (int i=0;i<this->query_parameters->db_->num_keywords;i++){
        s->noise_calculator->decrypt(selection_vector[i], pt);
        ASSERT_EQ(stoi(pt.to_string()), (i == keyword_index) ? 1 : 0);
    }
}

int main(int argc, char* argv[]){
    
    ::testing::InitGoogleTest(&argc, argv);
    int res = RUN_ALL_TESTS();

}

