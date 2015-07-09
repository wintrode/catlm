#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include "utils/vocab_trie.h"
#include "utils/zlibutil.h"

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
  char *outfile;

  file = argv[optind+1];
  outfile = argv[optind+2];

  char line[0x10000];
  FILE *infile=fopen(file, "r");
  int len = strlen(file);
  bool gzipped=(len > 3 && strncmp(file+len-3, ".gz",3)==0);
  if(gzipped) 
    init_gzip_stream(infile,&line[0]);
  while (readLine(infile,line,gzipped)) {
    if(line[0]==0)continue;// skip gzip new_block
    printf(line);
  }




}


