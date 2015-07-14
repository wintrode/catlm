// utils/utils.h

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

#ifndef CATLM_UTILS_VOCAB_TRIE_H
#define CATLM_UTILS_VOCAB_TRIE_H

#include <string>
#include <map>
#include <vector>


namespace catlm {

  using std::string;

  class VocabNode {
    
  public:
    int id;
    VocabNode *parent;
    std::map<string, VocabNode*> children;
    
    VocabNode(int vid, VocabNode *vparent);
    ~VocabNode();

  private:    

    
  };

  class VocabTrie {
    
  public:
    VocabTrie();
    ~VocabTrie();

    int load(const char* file);

    int insert(std::vector<const char*> &words);
    int insert(std::vector<const char*> &words, int id);
    
    int get_id(std::vector<const char*> &words);
    int get_id(std::vector<string> &words, int start, int len);

    void extract_vec(const char* text, std::map<int, double> &vec, int maxorder);
    int get_ngram_count() {
      return maxid+1;
    }
    
  private:
    int maxid;
    VocabNode *root;
    
    VocabNode *insert(VocabNode *here, std::vector<const char*> &words, int idx);
      
  };

}



#endif
