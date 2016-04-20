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

  typedef struct _latarc {
    int start;
    int end;
    int wid;
    int len;
    double cost;
  } latarc;


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

    bool trigOnly;

    int ngram_count;
    int num_utts;
    std::vector<NbestUtt > utts;

    int lc;

    int trigger_count;

    double lambda;

    int vsize;

  public:
    TriggerLM(VocabTrie &vt, int _order, int _min_order=1, int trig_ngram=0, bool fullng=false);
    ~TriggerLM();

    int get_ngram_count() { return ngram_count; }

    int get_ngram(int ngid, std::vector<std::string> & out, std::vector<int> &idout) { 
      vt.get_ngram(ngid, out, idout);
      return out.size();
    }

    int get_ngram_id(std::vector<std::string> &words, int pos, int len) {
      return vt.get_id(words, pos, len);
    }

    //int get_ngram_id(std::list<int> &words, int pos, int len) {
    //  return vt.get_id(words, pos, len);
    //}

    void reset_counters();

    int get_pcorrect() {
      return p.ccorrect;
    }
    int get_ptotal() {
      return p.tclass;
    }

    double get_alpha(int fid) {
      return p.get_alpha(fid);
    }

    int read_nbest_feats(FILE *infile, gzFile infd, 
                       std::vector<std::map<int, double> > &uvecs, 
                         std::vector<double> &scores, 
                         std::map<int, double> &hist,
			 std::vector<std::vector<int> > *nbestlist);

    int read_lattice_feats(FILE *infile, gzFile infd, 
                           std::vector<std::map<int, double> > &uvecs, 
                           std::vector<latarc> &arcs, 
                           std::map<int, double> &hist,
                           std::map<int, std::string> *vmap) ;

    string &get_key() { return current_key; }
    int train_example(std::map<int, double> &truth, 
                       std::vector<std::map<int, double> > &fvecs,
                       std::vector<double> &scores);

    int train_example_soft(std::vector<std::map<int, double> > &fvecs,
                           std::vector<double> errs, 
                           std::vector<double> &scores);

    int train_example_lattice(std::vector<std::map<int, double> > &fvecs,
                              std::vector<double> errs); 

    
    int rescore(std::vector<std::map<int, double> > &fvecs,
                std::vector<double> &scores);


    double rescore_lattice(std::map<int, double> &fvec);


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

    void setTrigOnly(bool b) {
      trigOnly=b;
    }

  };


  void print_vector(FILE*f, std::map<int, double> &vec); 
  void add_vector(std::map<int, double> &dest, std::map<int, double> &src,
                  int maxid); 
  void add_vector(std::map<int, double> &dest, std::map<int, double> &src,
                  double scalar);
}

#endif
