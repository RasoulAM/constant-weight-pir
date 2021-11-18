#include "database.h"

Database::Database(uint64_t __num_keywords, uint64_t __max_item_hex_length, uint64_t __MAX_KEYWORD_BITLENGTH){
    this->num_keywords = __num_keywords;
    this->max_item_hex_length = __max_item_hex_length;
    if (__num_keywords <= (1 << __MAX_KEYWORD_BITLENGTH)){
        this->MAX_KEYWORD_BITLENGTH = __MAX_KEYWORD_BITLENGTH;
    } else {
        this->MAX_KEYWORD_BITLENGTH = ceil(log2(this->num_keywords));
        cout << "keyword bitlength too short, changing to " << this->MAX_KEYWORD_BITLENGTH << endl;
    }
    this->generate_some_hex_data();
}

void Database::generate_some_hex_data(){
    srand(time(NULL));
    uint64_t _step = pow(2, this->MAX_KEYWORD_BITLENGTH - log2(this->num_keywords));
    for (int i=0;i<this->num_keywords;i++){
        
        uint64_t __new_keyword = i * _step;
        string __new_data_hex = gen_random_hex(this->max_item_hex_length, i);
        this->keywords.push_back(__new_keyword);
        this->data_hex_raw.push_back(__new_data_hex);
    }
}

uint64_t Database::get_random_query(){
    srand(time(NULL));
    uint64_t _rand_index = rand()%this->num_keywords;
    return this->keywords[_rand_index];
}

uint64_t Database::get_index_of_keyword(uint64_t keyword){
    for (int i=0;i<num_keywords;i++){
        if (this->keywords[i] == keyword){
            return i;
        }
    }
    return 0; // TODO: return something better in this case
}

vector<string> Database::get_hex_data_by_keyword(uint64_t keyword, uint64_t coefficient_hexlength){
    vector<string> _ans;
    uint64_t keyword_index = get_index_of_keyword(keyword);
    uint64_t __number_of_coeffs = ceil( (float) this->data_hex_raw[keyword_index].length() / coefficient_hexlength);
    for (int i=0;i<__number_of_coeffs;i++){
        _ans.push_back(this->data_hex_raw[keyword_index].substr(i*coefficient_hexlength, coefficient_hexlength));
    }
    return _ans;
}

vector<string> Database::get_hex_data_by_keyword_index(uint64_t keyword_index, uint64_t coefficient_hexlength){
    vector<string> _ans;
    uint64_t __number_of_coeffs = ceil( (float) this->data_hex_raw[keyword_index].length() / coefficient_hexlength);
    for (int i=0;i<__number_of_coeffs;i++){
        _ans.push_back(this->data_hex_raw[keyword_index].substr(i*coefficient_hexlength, coefficient_hexlength));
    }
    return _ans;
}