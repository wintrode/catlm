#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "corpus/corpus.h"


using namespace catlm;

void print_usage() {
  std::cerr << "Usage: catlm_train --topics T  --iterations N --order O "
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
    {"topics", required_argument, 0,  'n' },
    {"iterations", required_argument, 0,  'i' },
    {"order",      required_argument, 0,  'o' },
    {0,           0,                  0,  0   }
  };
  
  int long_index =0;
  int opt = 0;
  while ((opt = getopt_long(argc, argv,"n:i:o:", 
			    long_options, &long_index )) != -1) {
    

    switch (opt) {
    case 'n' : ntopics = atoi(optarg); 
      break;
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
  
  fname = argv[optind];
  outpref = argv[optind+1];

  string s(fname);
  
  Corpus corp(s);

  corp.debug_stats();
  
  // write-out vocab
  if (outpref != NULL) 
    corp.vocabulary()->save(s = (string(outpref) + ".vocab"));

  // initialize topic model
  /*  CacheTopicModel ctm = (corp.vocabulary(), ntopics, cache_order);

  ctm.estimate(corp, iterations);
  
  ctm.write(s=(string(outpref) + ".model"));*/
  // learn topics

  // output topic model state....

  
  return 0; 
}
