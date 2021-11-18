#include "utils.h"

string gen_random_hex() {
    static const char hex_letters[] = "0123456789ABCDEF";
    string s(2, ' ');
    for (int i = 0; i < 2; ++i) {
        s[i] = hex_letters[rand() % (sizeof(hex_letters)-1)];
    }
    s[2] = 0;
    return s;
}

string gen_random_hex(int length) {
    static const char hex_letters[] = "123456789ABCDEF";
    string s(length, ' ');
    for (int i = 0; i < length; ++i) {
        s[i] = hex_letters[rand() % (sizeof(hex_letters)-1)];
    }
    s[length] = 0;
    return s;
}

string gen_random_hex(int length, int x) {
    static const char hex_letters[] = "123456789ABCDEF";
    string s(length, ' ');
    for (int i = 0; i < length; ++i) {
        s[i] = hex_letters[x % (sizeof(hex_letters)-1)];
    }
    s[length] = 0;
    return s;
}

std::string vector_to_poly_string(vector<uint64_t> coeffs){
    chrono::high_resolution_clock::time_point time1 = chrono::high_resolution_clock::now();
    string ans = int_to_hex((int)(coeffs[0]));
    for (int i=1;i<coeffs.size();i++){
        ans = int_to_hex((int)(coeffs[i])) + "x^" + to_string(i) + " + " + ans;
    }
    return ans;
}

std::string hex_vector_to_poly_string(vector<string> coeffs){
    string ans = coeffs[0];
    for (int i=1;i<coeffs.size();i++){
        ans = coeffs[i] + "x^" + to_string(i) + " + " + ans;
    }
    return ans;
}

uint64_t hex_string_to_uint(string str){
    uint64_t x;   
    std::stringstream ss;
    ss << std::hex << str;
    ss >> x;
    return x;
}

pair<uint64_t, string> monomial_to_hex(string monomial){
    uint64_t index;
    string coeff;
    if (monomial.find("x^") != string::npos){
        size_t hit = monomial.find("x^");
        index=stoi(monomial.substr(hit+2));
        coeff = monomial.substr(0, hit);
    } else if (monomial.find("x") != string::npos){
        size_t hit = monomial.find("x");
        index=1;
        coeff = monomial.substr(0, hit);
    } else{
        index=0;
        coeff=monomial;
    }
    return pair<uint64_t, string>(index, coeff);
}

pair<uint64_t, uint64_t> monomial_to_integer(string monomial){
    uint64_t index, coeff;
    if (monomial.find("x^") != string::npos){
        size_t hit = monomial.find("x^");
        index=stoi(monomial.substr(hit+2));
        coeff = hex_string_to_uint(monomial.substr(0, hit));
    } else if (monomial.find("x") != string::npos){
        size_t hit = monomial.find("x");
        index=1;
        coeff = hex_string_to_uint(monomial.substr(0, hit));
    } else{
        index=0;
        coeff=hex_string_to_uint(monomial);
    }
    return pair<uint64_t, uint64_t>(index, coeff);
}

vector<uint64_t> poly_string_to_vector(std::string poly_string, uint64_t max_size){
    string __temp = poly_string;
    vector<uint64_t> ans(max_size, 0);
    pair<uint64_t, uint64_t> mono;
    while (__temp.find("+") != string::npos){
        mono = monomial_to_integer(__temp.substr(0, __temp.find("+")));
        ans[mono.first] = mono.second;
        __temp = __temp.substr(__temp.find("+")+1);
    }
    mono = monomial_to_integer(__temp);
    ans[mono.first] = mono.second;
    return ans;
}

vector<uint64_t> poly_string_to_hex_vector(std::string poly_string, uint64_t max_size){
    string __temp = poly_string;
    vector<uint64_t> ans(max_size, 0);
    pair<uint64_t, string> mono;
    while (__temp.find("+") != string::npos){
        mono = monomial_to_hex(__temp.substr(0, __temp.find("+")));
        ans[mono.first] = hex_string_to_uint(mono.second);
        __temp = __temp.substr(__temp.find("+")+1);
    }
    mono = monomial_to_hex(__temp);
    ans[mono.first] = hex_string_to_uint(mono.second);
    return ans;
}

uint64_t mod_exp(uint64_t a, uint64_t e, uint64_t n){
    if (e == 0) return 1;

    long k = ceil(log2(e));
    uint64_t res;
    res = 1;

    for (long i = k-1; i >= 0; i--) {
        res = (res*res) % n;
        if ((e/(uint64_t)(pow(2,i)))%2 == 1)
            res = (res*a) % n;
    }
    return res;
}

uint64_t prime_mod_inverse(uint64_t a, uint64_t n){
    return mod_exp(a, n-2, n);
}

uint64_t iter_factorial(uint64_t n){
    uint64_t ret = 1;
    for(uint64_t i = 1; i <= n; ++i)
        ret *= i;
    return ret;
}

uint64_t choose(uint64_t n, uint64_t k) {
    if (k > n) {
        return 0;
    }
    uint64_t r = 1;
    for (uint64_t d = 1; d <= k; ++d) {
        r *= n--;
        r /= d;
    }
    return r;
}

uint64_t find_choose(uint64_t k, uint64_t n){
    uint64_t _ans = k;
    while (choose(_ans, k) < n){
        _ans++;
    }
    return _ans;
}


float log2_choose(uint64_t n, uint64_t k){
    if (k > n) {
        cout << "Can't have k > n" << endl;
        return -1; // Error case
    }
    float log2_r = 0;
    for (uint64_t d = 1; d <= k; ++d) {
        log2_r += log2(n) - log2(d);
        n--;
    }
    return log2_r;
}

uint64_t find_log2_choose(uint64_t k, uint64_t log_n){
    uint64_t _ans = k;
    while (log2_choose(_ans, k) < log_n){
        _ans++;
    }
    return _ans;
}

/*
    return smallest power of 2 bigger than x
*/
uint64_t bigger_power2(uint64_t x){
    uint64_t ans=1;
    while (ans < x) ans*=2;
    return ans;
}

std::vector<uint64_t> get_perfect_constant_weight_codeword(uint64_t __number, uint64_t encoding_size, uint64_t hamming_weight, bool __verbose){
    vector<uint64_t> ans(encoding_size, 0ULL);
    uint64_t mod_size = choose(encoding_size, hamming_weight);
    if (__number >= mod_size){
        if (__verbose) cout << "Overflow occurred, everything okay?" << endl;
        __number %= mod_size;
    }
    long remainder = __number, k_prime = hamming_weight;
    for (long pointer=encoding_size-1; pointer>=0; pointer--){
        if (remainder >= choose(pointer, k_prime)){
            ans[pointer] = 1ULL;
            remainder -= choose(pointer, k_prime);
            k_prime -= 1;
        }
    }
    return ans;
}

std::vector<uint64_t> get_folklore_encoding(uint64_t __number, uint64_t encoding_size, uint64_t hamming_weight, bool __verbose){
    vector<uint64_t> ans(encoding_size, 0ULL);
    uint64_t mod_size = pow(2, encoding_size);
    if (__number >= mod_size){
        if (__verbose) cout << "Overflow occurred, everything okay?" << endl;
        __number %= mod_size;
    }
    long remainder = __number;
    for (long pointer=0; pointer<encoding_size; pointer++){
        ans[pointer] = remainder % 2;
        remainder /= 2;
    }
    return ans;
}