#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <istream>
#include <fstream>
#include <string.h>

#include "utils/vocab_trie.h"
#include "utils/utils.h"
#include "asr/triglm.h"

using namespace catlm;
using namespace std;

void print_usage() {
  std::cerr << "Usage: triglm_rescore --iterations N --order O "
	   "<nbest> <nbest>\n";

}

int main(int argc, char **argv) { 
  char *fname = NULL;
  char *outpref = NULL;
  int cache_order = 1;
  int min_order=1;
  int niterations = 1000;

  char *lm = NULL;

  //Specifying the expected options
  //The two options l and b expect numbers as argument
  static struct option long_options[] = {
    {"order",      required_argument, 0,  'o' },
    {"min-order",      required_argument, 0,  'O' },
    {"lm",      required_argument, 0,  'l' },
    {0,           0,                  0,  0   }
  };
  
  int long_index =0;
  int opt = 0;
  while ((opt = getopt_long(argc, argv,"o:l:O:", 
			    long_options, &long_index )) != -1) {
    

    switch (opt) {
    case 'o' : cache_order = atoi(optarg);
      break;
    case 'O' : min_order = atoi(optarg);
      break;
    case 'l' : lm = optarg;
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

  char *model = argv[optind];
  char *nbest = argv[optind+1];

  if (lm == NULL) {
    std::cerr << "Must specify LM currently to get ngram id's\n";
    return -1;
  }

  VocabTrie vt;
  int ngram_count = vt.load(lm);  

  vector<const char*> unigram;
  unigram.resize(1, NULL);

  unigram[0] = "<eps>";
  vt.insert(unigram);
  unigram[0] = "<unk>";
  vt.insert(unigram);


  TriggerLM tlm(vt, cache_order, min_order);


  // do I read the n-best file in once or at each iteration...
  FILE *infile=NULL;
  gzFile infd = 0;
  int len = strlen(nbest);
  bool gzipped=(len > 3 && strncmp(nbest+len-3, ".gz",3)==0);

    int n;

    
  if(gzipped) 
    infd = gzopen(nbest, "r");
  else
    infile = fopen(nbest, "r");
  
  
  // open file, next_nbest_features(vector of feature vectors, with scores)
  vector<map<int, double> > uvecs;
  vector<double> scores;
  int nb = 0;
  int count =0 ;
  string key;
  
  
  tlm.read(model, NULL);
  int i;
  double st, end;
  NbestUtt *utt;
  int idlen;
  const char *idptr;
  //  tlm.debug();
  map<int, double> history_vec;
  vector<vector<int> > nbestlist;
  cerr << "Iteraton " << n << "\n";
  tlm.save_utts();
  while ( (nb = tlm.read_nbest_feats(infile, infd, uvecs, scores, history_vec, 0)) > 0) {

    //    printf("# original utterance - %f\n", n, scores[0]);
    idptr = strrchr(tlm.get_key().c_str(), '_');
    idlen = idptr-tlm.get_key().c_str();
    if (strncmp(key.c_str(), tlm.get_key().c_str(), idlen))
      history_vec.clear();

    n = tlm.rescore(uvecs, scores);
    //printf("# selected utterance %d %f\n", n, scores[n]);
    key = tlm.get_key();
    st = 0.0; 
    utt = tlm.get_utt(n);
    for (i=0; i < utt->wids.size(); i++) {
      end = st+ (double)utt->lens[i]/100.0;
      printf("%s 1 %0.2f %0.2f %s %0.2f\n", key.c_str(), st, end-st, 
             vt.get_word(utt->wids[i]).c_str(), 0.99 );//utt->scores[i]);
      st = end;
             
    }
    
    add_vector(history_vec, uvecs[n], vt.get_ngram_count());

  }
  

  if (gzipped)
    gzclose(infd);
  else
    fclose(infile);
  
  
}



