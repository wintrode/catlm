// topics/topicmodel.h

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

#ifndef CATLM_TOPICS_TOPICMODEL_H
#define CATLM_TOPICS_TOPICMODEL_H


#include <string>
#include <map>
#include <vector>

#include "../corpus/corpus.h"


namespace catlm {

  using std::string;

  class CacheTopicModel {
  private:
    int ntopics;
    int cache_order;
    Vocabulary *vocab;
    
    void init();
    int initFromCorpus(Corpus *corpus);
    void sample(Document *doc);

    // sampling variables
    int burnin;
    int opt_interval;

    void optimize_alpha();
    void optimize_beta();
    void optimize_nu();

    long runtime;
    long innerRuntime;


    bool printTiming;

  public:
    
  CacheTopicModel(Vocabulary *_vocab, int _ntopics, int _cache_order) :
    vocab(_vocab), ntopics(_ntopics), cache_order(_cache_order) {
      init();
      
    }

    ~CacheTopicModel();

    int estimate(Corpus *corpus, int niterations);

    int write(const char *fname);
    
    double modelLogLikelihood();

  };


}


#endif 
