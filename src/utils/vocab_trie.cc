#include "vocab_trie.h"


using namespace catlm;

VocabNode::VocabNode(int vid) : id (vid) {
    
}

VocabTrie::VocabTrie() {

    root = new VocabNode(-1);

}
