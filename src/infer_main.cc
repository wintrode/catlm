#include <stdio.h>
#include "corpus/corpus.h"

using namespace catlm;

int main(int argc, char **argv) { 
  char *fname = NULL;
  char *inpref = NULL;
  if (argc > 2) {
    fname = argv[1];
    inpref = argv[2];
  }
  else
    return -1;

  string s(fname);
  
  string v(string(inpref) + ".vocab");

  Vocabulary vocab(v);

  Corpus corp(s,vocab);  // can still specify add=T or F here

  corp.debug_stats();
  
  return 0; 
}
