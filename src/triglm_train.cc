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
  std::cerr << "Usage: triglm_train --iterations N --order O "
	   "<input-dir> <outpref>\n";

}

inline double  min3(double v1, double v2, double v3) {
  if (v1 <= v2 && v1 <= v3) 
    return v1;
  if (v2 <= v1 && v2 <= v2)
    return v2;
  if (v3 <= v1 && v3 <= v2)
    return v3;
  
  return v1;
}

double get_error(vector<int> &ref, vector<int> &hyp) {
  double err = 0.0;
  double **dp;
  dp = new double*[ref.size()+1];
  int i,j;
  for (i=0; i <= ref.size(); i++) {
    dp[i] = new double[hyp.size()+1];
    if (i==0) {
      dp[0][0]=0.0;
      for (j=1; j <= hyp.size(); j++) 
	dp[0][j]=(double)j;
    }
    else {
      dp[i][0]=(double)i;
    }
  }
  double cost=0.0;
  for (i=1; i <= ref.size(); i++) {
    for (j=1; j <= hyp.size(); j++) {
      if (ref[i-1]==hyp[j-1]) // correct
	cost = 0;
      else
	cost = 1.0;
      dp[i][j] = min3(dp[i-1][j-1]+cost, dp[i][j-1]+1, dp[i-1][j]+1);
    }
  }

  err = dp[ref.size()][hyp.size()];

  /*for (i=0; i <= ref.size(); i++)  {
    for (j=0; j <= hyp.size(); j++) {
      fprintf(stderr, " %0.1f", dp[i][j]);
    }
    fprintf(stderr, "\n");

    delete [] dp[i];
  }
  */

  delete [] dp;

  return err;
  
  
}

int main(int argc, char **argv) { 
  char *fname = NULL;
  char *outpref = NULL;
  int cache_order = 1;
  int min_order = 1;
  int ntopics = 50;
  int niterations = 1000;

  char *model = NULL;
  double alpha0 = 1.0;
  bool debug=false;
  bool triggers=false;

  //Specifying the expected options
  //The two options l and b expect numbers as argument
  static struct option long_options[] = {
    {"iterations", required_argument, 0,  'i' },
    {"order",      required_argument, 0,  'o' },
    {"min-order",  required_argument, 0,  'O' },
    {"model",      required_argument, 0,  'm' },
    {"debug",      no_argument,       0,  'd' },
    {"triggers",   no_argument,       0,  't' },
    {"alpha",      required_argument, 0,  'a' },
    {0,           0,                  0,  0   }
  };
  
  int long_index =0;
  int opt = 0;
  while ((opt = getopt_long(argc, argv,"i:o:m:da:O:t", 
			    long_options, &long_index )) != -1) {
    

    switch (opt) {
    case 'i' : niterations = atoi(optarg);
      break;
    case 'o' : cache_order = atoi(optarg);
      break;
    case 'O' : min_order = atoi(optarg);
      break;
    case 'a' : alpha0 = strtod(optarg, 0);
      break;
    case 'm' : model = optarg;
      break;
    case 'd' : debug = true;
      break;
    case 't' : triggers = true;
      break;
    default: 
      std::cerr << optarg <<"\n";
      print_usage(); 
      exit(EXIT_FAILURE);
    }
  }

  if (optind + 2 >= argc) {
    std::cerr << "expecting 2 more arguments\n";  
    return -1;
  }

  char *lm = argv[optind];
  char *train_text = argv[optind+1];
  char *nbest = argv[optind+2];

  printf("Nbest %s train %s\n", nbest, train_text);

  VocabTrie vt;
  int ngram_count = vt.load(lm);

  std::map<string, std::vector<int> > training;
  std::map<string, map<int, double> > trexamples;

  cerr << "opening " << train_text << "\n";
  
  std::istream *f;
  if (!strcmp(train_text, "-")) {
    f = &std::cin;
  }
  else {
    f = new std::ifstream(train_text);
  }

  std::string line, key;
  std::vector<int> wids;
  std::vector<string> words;
  std::vector<const char*> unigram;
  unigram.resize(1, NULL);
  int id;
  int i;

  unigram[0] = "<eps>";
  vt.insert(unigram);
  unigram[0] = "<unk>";
  vt.insert(unigram);

  TriggerLM tlm(vt, cache_order, min_order, triggers);

  map<int, double> trvec;

  while (getline( *f, line )) {
    wids.resize(0);
    split(line, ' ', words);
    key = words[0];
    trvec.clear();
    int n,id;
    // add all the 1-order in this utterance into the vector
    for (i=1; i < words.size(); i++) {
      for (n=1; n <= cache_order; n++) {  // do 1 thru order grams
        if (i == words.size() - (n-1) )
          continue;
        id = vt.get_id(words, i, n);
        if (n==1 && id >= 0)
            wids.push_back(id);

        if (n < min_order)
          continue; // skip unigrams??

        if (trvec.count(id) > 0) 
          trvec[id+1] = trvec[id+1]+1.0;
        else 
          trvec[id+1] = 1.0;
        
      }
    }

    // what should  trvec[0] be?

    training[key]=wids;
    trexamples[key]=trvec;
    //print_vector(stderr, trvec);

  }

  if (f != & std::cin) {
    ((std::ifstream*)f)->close();
    delete f;
  }
  


  if (alpha0 > 0.0)
    tlm.setAlpha0(alpha0);


  // do I read the n-best file in once or at each iteration...
  FILE *infile=NULL;
  gzFile infd = 0;
  int len = strlen(nbest);
  bool gzipped=(len > 3 && strncmp(nbest+len-3, ".gz",3)==0);

  cerr << "Training for " << niterations << "\n";
  int n;
  for (n=0; n < niterations; n++) {
    
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

    tlm.reset();
    vector<vector<int> > nbestlist;
    cerr << "Iteraton " << n << "\n";
    tlm.reset_counters();
    
    int idlen;
    const char *idptr;

    int mini;
    double minerr=1000.0; 
    double err = 0.0;
    map<int,double> history_vec;
    
    while ( (nb = tlm.read_nbest_feats(infile, infd, uvecs, scores, history_vec, &nbestlist)) > 0) {
      //fprintf(stderr, "Found %d-best utts for %s\n", nb, tlm.get_key().c_str());
      
      mini = 0;
      minerr = 1000;
      for (i=0; i < nb; i++) {
	err = get_error(training[tlm.get_key()], nbestlist[i]);
        //fprintf(stderr, "Found min %s-%d : %f\n", tlm.get_key().c_str(), i, err);
	if (err < minerr) {
	  mini = i; minerr = err;
	}
      }
      //fprintf(stderr, "Found min %s-%d : %f\n", tlm.get_key().c_str(), mini, minerr);
      idptr = strrchr(tlm.get_key().c_str(), '_');
      idlen = idptr-tlm.get_key().c_str();
      if (strncmp(key.c_str(), tlm.get_key().c_str(), idlen))
        history_vec.clear();

      //i = tlm.train_example(trexamples[tlm.get_key()], uvecs, scores);
      //fprintf(stderr, "bow found %s-%d\n", tlm.get_key().c_str(), i);
      i = tlm.train_example(uvecs[mini], uvecs, scores);
      count++;
      if (count%1000==0 )
        cerr << ".";
      
      if (count % 10000==0)
        cerr << count  << "\n";

      add_vector(history_vec, uvecs[i], vt.get_ngram_count());

      key = tlm.get_key();
    }


    if (gzipped)
      gzclose(infd);
    else
      fclose(infile);
        

  }

  if (debug)
    tlm.debug();

  // for each segment, 
  // extract n=gram vector
  // score loss function and update vector
  
  if (model)
    tlm.write(model, false);


}


// decode eval2000, dev2h, and generate 100-best lists

// test 500best with new code
// get WER for nbest-0 entry 

// write down a-values

// implement trigger features
