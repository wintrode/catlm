#include "vocab_trie.h"


using namespace catlm;

using std::string;

VocabNode::VocabNode(int vid, VocabNode *vparent) : id (vid), parent (vparent) {
  // nothign to be done?
}

VocabNode::~VocabNode() {
  std::map<string,VocabNode*>::iterator it;
  for (it = children.begin(); it != children.end(); it++) {
    delete it->second;
  }
}


VocabTrie::VocabTrie() {
  root = new VocabNode(-1, (VocabNode*)NULL);
  maxid = -1;
}


int VocabTrie::insert(std::vector<string> &words) {
  int pos;
  VocabNode *node = root;
  for (pos=0; pos < words.size(); pos++) {
    node = insert(node, words, pos);
  }
}


VocabNode *VocabTrie::insert(VocabNode *here, std::vector<string> &words, int idx) {
  std::map<string,VocabNode*>::iterator it;
  it = here->children.find(words[idx]);

  if (it != here->children.end()) // word already in trie
    return it->second;
  
  VocabNode *node = new VocabNode(++maxid, here);
  here->children[words[idx]]=node;
  return node;
}


int VocabTrie::get_id(std::vector<string> &words) {
  VocabNode *node = root;
  int pos = 0;
  std::map<string,VocabNode*>::iterator it;

  while (node && pos < words.size()) {
    if (pos == words.size()) 
      return node->id;

    it = node->children.find(words[pos]);
    if (it == node->children.end())
      break; // not found

    node = it->second;
      
  }
  
  return -1;
  

}
