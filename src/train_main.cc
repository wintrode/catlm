#include <stdio.h>
#include "corpus/corpus.h"


using namespace catlm;



int main(int argc, char **argv) { 
  char *fname = NULL;
  char *outpref = NULL;
  if (argc > 2) {
    fname = argv[1];
    outpref = argv[2];
  }
  

  if (fname == NULL) 
    return -1;

  string s(fname);
  
  Corpus corp(s);

  corp.debug_stats();
  
  // write-out vocab
  if (outpref != NULL) 
    corp.vocabulary()->save(s = (string(outpref) + ".vocab"));

  // initialize topic model
  
  // learn topics

  // output topic model state....

  
  return 0; 
}
