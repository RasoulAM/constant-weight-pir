// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.
  
#include <stdio.h>  
#include <unistd.h>  
#include <getopt.h>  

#include "pirutils.h"
#include "database.h"
#include "PIRserver.h"
#include "client.h"

using namespace std;

void simple_PIR(
        uint64_t hamming_weight,
        EqualityType eq_type,
        uint64_t log_poly_mod_degree,
        uint64_t number_of_keywords,
        uint64_t max_per_row_payload_byte_size,
        uint64_t keyword_bitlength,
        bool __verbose,
        string write_path,
        uint64_t num_threads
    ){

    if (keyword_bitlength <= 0){
        keyword_bitlength = (uint64_t)(ceil(log2(number_of_keywords)));
    }

    Database db(number_of_keywords, max_per_row_payload_byte_size*2, keyword_bitlength);
    QueryParameters* query_parameters = new QueryParameters(log_poly_mod_degree, eq_type, hamming_weight, &db, keyword_bitlength, num_threads);
    query_parameters->write_path = write_path;

    Client c;
    bool _result =  c.end_to_end_PIR_query(query_parameters, __verbose);

}

void show_usage(){
    cout << "See how the code works" << endl;
}

int main(int argc, char *argv[]){ 
    uint64_t hamming_weight=2;
    uint64_t log_poly_mod_degree=13;
    EqualityType eq_type=Constant_Weight;

    uint64_t number_of_keywords=100;
    uint64_t max_per_row_payload_byte_size=0;
    uint64_t keyword_bitlength=0;
    uint64_t num_threads=0;
    bool verbose=true;
    string write_path = "";

    if (argc < 2){
        show_usage();
        return 0;
    }

    const char * type = argv[1];

    int opt;
    while(1){

        int option_index = 0;
        static struct option long_options[] = 
        {
            {"hamming_weight",       optional_argument, NULL,  'h' },
            {"eq_type",              optional_argument, NULL,  'e' },
            {"log_poly_mod_degree",  optional_argument, NULL,  'd' },
            {"num_keywords",         optional_argument, NULL,  'n' },
            {"keyword_bitlength",    optional_argument, NULL,  'x' },
            {"response_bytesize",    optional_argument, NULL,  'n' },
            {"verbose",              no_argument,       NULL,  'v' },
            {"write_path",           optional_argument, NULL,  'w' },
            {"num_threads",          optional_argument, NULL,  't' },
            {NULL,                   0,                 NULL,   0  }
        };

        opt = getopt_long(argc, argv, "h:e:d:n:s:m:b:a:x:w:t:vc", long_options, &option_index);
        if (opt == -1)
            break;

        switch(opt){
            case 'h':
                hamming_weight = stoi(optarg);
                break;
            case 'e':
                eq_type = stoi(optarg) == 0 ? Folklore : Constant_Weight;
                break;
            case 'd':
                log_poly_mod_degree = stoi(optarg);
                break;
            case 'n':
                number_of_keywords = stol(optarg);
                break;
            case 'x':
                keyword_bitlength = stol(optarg);
                break;
            case 's':
                max_per_row_payload_byte_size = stoi(optarg);
                break;
            case 'v':
                verbose = false;
                break;
            case 'w':
                write_path = optarg;
                break;
            case 't':
                num_threads = stoi(optarg);
                break;
            case ':':  
                printf("option needs a value\n");  
                break;
            case '?':
                printf("unknown option: %c\n", optopt); 
                break; 
        }
    }

    // This is a defalut value which will fill each ciphertext
    if (max_per_row_payload_byte_size <= 0)
        max_per_row_payload_byte_size = (1 << log_poly_mod_degree)*5/ 2;

    std::cout << "Performing Simple PIR ..." << endl;
    simple_PIR(hamming_weight, eq_type, log_poly_mod_degree, number_of_keywords, max_per_row_payload_byte_size, keyword_bitlength, verbose, write_path, num_threads);

}