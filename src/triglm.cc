#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include "utils/vocab_trie.h"


using namespace catlm;

void print_usage() {
  std::cerr << "Usage: triglm_train --iterations N --order O "
	   "<input-dir> <outpref>\n";

}

int main(int argc, char **argv) { 
  char *fname = NULL;
  char *outpref = NULL;
  int cache_order = 1;
  int ntopics = 50;
  int niterations = 1000;

  //Specifying the expected options
  //The two options l and b expect numbers as argument
  static struct option long_options[] = {
    {"iterations", required_argument, 0,  'i' },
    {"order",      required_argument, 0,  'o' },
    {0,           0,                  0,  0   }
  };
  
  int long_index =0;
  int opt = 0;
  while ((opt = getopt_long(argc, argv,"i:o:", 
			    long_options, &long_index )) != -1) {
    

    switch (opt) {
    case 'i' : niterations = atoi(optarg);
      break;
    case 'o' : cache_order = atoi(optarg);
      break;
    default: 
      std::cerr << optarg <<"\n";
      print_usage(); 
      exit(EXIT_FAILURE);
    }
  }

  if (optind + 1 >= argc) {
    std::cerr << "expecting 2 more arguments\n";  
    return -1;
  }

}


