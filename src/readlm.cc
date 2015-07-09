#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include "utils/vocab_trie.h"


#include <string.h>

using namespace catlm;

void print_usage() {
  std::cerr << "Usage: readlm [--order 3] "
	   "<input> <output>\n";

}

int main(int argc, char **argv) { 
  char *fname = NULL;
  char *outpref = NULL;
  int order = 3;

  VocabTrie vt;


  //Specifying the expected options
  //The two options l and b expect numbers as argument
  static struct option long_options[] = {
    {"order",      required_argument, 0,  'o' },
    {0,           0,                  0,  0   }
  };
  
  int long_index =0;
  int opt = 0;
  while ((opt = getopt_long(argc, argv,"o:", 
			    long_options, &long_index )) != -1) {
    

    switch (opt) {
    case 'o' : order = atoi(optarg);
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

  char *file;
  char *text;

  file = argv[optind];
  text = argv[optind+1];

  int ng = vt.load(file);
  printf("Processed %d n-grams\n", ng);

  FILE* tf = fopen(text, "r");
  
  std::map<int, double> vec;

  char buf[1000];
  char *ptr=0;
  while (fgets(buf, 999, tf)) {
    ptr = strchr(buf,'\n');
    if (ptr)
      *ptr=0;
    //printf("%s\n", buf);

    vt.extract_vec(buf, vec, 3);
  }

  fclose(tf);

}


