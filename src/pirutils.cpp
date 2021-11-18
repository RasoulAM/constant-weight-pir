#include "pirutils.h"

QueryParameters::QueryParameters(uint64_t log_poly_mod_degree, EqualityType eq_type, uint64_t hamming_weight, Database* db, uint64_t keyword_bitlength, uint64_t num_threads){
    this->init(log_poly_mod_degree, eq_type, hamming_weight, db, num_threads);
    this->set_db_parameters();
    this->keyword_bitlength = keyword_bitlength;
    this->set_query_specific_parameters();
}

void QueryParameters::init(uint64_t log_poly_mod_degree, EqualityType eq_type, uint64_t hamming_weight, Database* db_, uint64_t num_threads){
    this->db_=db_;
    this->log_poly_mod_degree=log_poly_mod_degree;
    this->poly_mod_degree=1<<this->log_poly_mod_degree;
    this->coefficient_hexlength = this->prime_bitlength / 4;
    this->hamming_weight=hamming_weight;
    this->num_threads = num_threads;
    this->eq_type = eq_type;
}

void QueryParameters::print_db_specific_parameters(){
    cout << "--------------------- Database Specific Parameters ---------------------" << endl
            << "\tNumber of Keywords: " << this->db_->num_keywords << endl
            << "\tKeyword Max Bitlength: " << this->db_->MAX_KEYWORD_BITLENGTH << endl
            << "\tMax Item Hex Length: " << this->db_->max_item_hex_length << endl
            << "\tResponse Cipher Count: " << this->num_output_ciphers << endl
            << "--------------------- Query Specific Parameters ---------------------" << endl
            << "\tEquality Type: " << ((this->eq_type == Constant_Weight) ? "Constant Weight" : "Folklore")  << endl
            << "\tHamming Weight: " << this->hamming_weight << endl
            << "\tCompression Factor: " << this->compression_factor << endl
            << "\tEncoding size: " << this->encoding_size << endl
            << "\tNumber of Input Ciphers: " << this->num_input_ciphers << endl
            << "--------------------------------------------------------------------" << endl;

}

void QueryParameters::add_parameters_to_metrics(){

    this->metrics_["num_keywords"] = this->db_->num_keywords;
    this->metrics_["max_item_hex_length"] = this->db_->max_item_hex_length;
    this->metrics_["num_output_ciphers"] = this->num_output_ciphers;
    this->metrics_["eq_type"] = this->eq_type;
    this->metrics_["hamming_weight"] = this->hamming_weight;
    this->metrics_["compression_factor"] = this->compression_factor;
    this->metrics_["encoding_size"] = this->encoding_size;
    this->metrics_["num_input_ciphers"] = this->num_input_ciphers;
    this->metrics_["num_threads"] = num_threads;

    this->metrics_["log_poly_mod_degree"] = this->log_poly_mod_degree;
    this->metrics_["minus_log2_false_positive_rate"] = 0;
    this->metrics_["max_bins_load"] = 0;

}

void QueryParameters::write_results_to_file(){
    ofstream output_file;
    string filename=this->write_path + to_string(chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count() % 100000000000);
    output_file.open( filename + ".csv");
    for (pair<string, uint64_t> metric : this->metrics_){
        output_file << metric.first << ", " << metric.second << endl;
    }
    output_file.close();
}

void QueryParameters::set_db_parameters(){
    this->num_output_ciphers = ceil((float)ceil(((float)this->db_->max_item_hex_length / this->coefficient_hexlength)) / this->poly_mod_degree);
}

void QueryParameters::set_query_specific_parameters(){

    switch (this->eq_type) {
        case Folklore:
            this->encoding_size = this->keyword_bitlength;
            break;
        case Constant_Weight:
            this->encoding_size = hamming_weight;
            while(choose(this->encoding_size,this->hamming_weight) < (1 << this->keyword_bitlength)) {
                this->encoding_size++;
            };
            break;
    }

    this->ceil_log2_encoding_size=ceil(log2(this->encoding_size));
    this->compression_factor=min(this->ceil_log2_encoding_size,this->log_poly_mod_degree);
    this->used_slots_per_pt = 1 << this->compression_factor;
    this->num_input_ciphers = ceil((float)this->encoding_size / (1<<this->compression_factor));
}

vector<uint64_t> QueryParameters::get_encoding_of_(uint64_t __keyword){
    vector<uint64_t> ans;
    switch (this->eq_type)
    {
        case Folklore:
            ans = get_folklore_encoding(__keyword, this->encoding_size, this->hamming_weight);
            break;
        case Constant_Weight:
            ans = get_perfect_constant_weight_codeword(__keyword, this->encoding_size, this->hamming_weight);
            break;
    }
    return ans;
}
