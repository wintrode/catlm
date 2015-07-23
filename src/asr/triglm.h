// asr/nbest.h

// Copyright 2015 Jonathan Wintrode


// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#ifndef CATLM_ASR_TRIGLM_H
#define CATLM_ASR_TRIGLM_H

#include <vector>
#include <map>

#include <stdio.h>

#include "../utils/zlibutil.h"
#include "../utils/vocab_trie.h"
#include "../classifiers/perceptron.h"

#define TRIGLM_VER 1

namespace catlm {

  class NbestUtt {
  public:
    std::vector<int> wids;
    std::vector<double> scores;
    std::vector<int> lens;
  };


  class TriggerLM {

  private:
    VocabTrie &vt;
    int order;
    int min_order;
    string lastkey;
    string current_key;

    bool use_triggers;
    Perceptron p;

    bool saveUtts;

    int ngram_count;
    int num_utts;
    std::vector<NbestUtt > utts;

    int lc;

  public:
    TriggerLM(VocabTrie &vt, int _order, int _min_order=1, bool triggers=false);
    ~TriggerLM();

    void reset_counters();

    int read_nbest_feats(FILE *infile, gzFile infd, 
                       std::vector<std::map<int, double> > &uvecs, 
                         std::vector<double> &scores, 
                         std::map<int, double> &hist,
			 std::vector<std::vector<int> > *nbestlist);

    string &get_key() { return current_key; }
    int train_example(std::map<int, double> &truth, 
                       std::vector<std::map<int, double> > &fvecs,
                       std::vector<double> &scores);
    
    int rescore(std::vector<std::map<int, double> > &fvecs,
                std::vector<double> &scores);


    void save_utts() { saveUtts=true;}
    NbestUtt* get_utt(int i);
    void write(const char *file, bool writeng);
    bool read(const char* file, const char* lm);
    void debug();
    void reset();
    
    void setAlpha0(double alpha0) {
      p.setFixedA0(true);
      p.setAlpha0(alpha0);
    }

  };


  void print_vector(FILE*f, std::map<int, double> &vec); 
  void add_vector(std::map<int, double> &dest, std::map<int, double> &src,
                  int maxid); 
}

#endif
