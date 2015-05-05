#include "corpus.h"

#include "../utils/utils.h"

using namespace catlm;

Vocabulary::Vocabulary() {
  
}

Vocabulary::Vocabulary(std::string &fname) {

  read_map(fname, idmap, &vocab);
  
}

Vocabulary::~Vocabulary() {
  
}

void Vocabulary::load(std::string &fname) {
  idmap.clear();
  vocab.resize(0);
  read_map(fname, idmap, &vocab);
}


void Vocabulary::save(std::string &fname) {
  // 
}
