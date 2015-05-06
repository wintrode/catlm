#include "corpus.h"

#include "../utils/utils.h"
#include <boost/algorithm/string.hpp>

#include <dirent.h>

using namespace catlm;

Vocabulary::Vocabulary() : unk_id(-1), unk("<unk>") {
  vocab.clear();
  idmap.clear();
}

Vocabulary::Vocabulary(std::string &fname) 
  : unk_id(-1), unk("<unk>") {

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
  write_vector(fname, vocab); 
}



// Document class
Document::Document(std::vector<int> &tokens) {
  int i=0;
  for (i=0; i < tokens.size(); i++ ) {
    this->tokens.push_back(tokens[i]);
  }
}

Document::Document(std::string &file, Vocabulary *vocab, bool add=true) {
  catlm::load_utf8_text(file, tokens, vocab, add);
}


Document::~Document() {
  // STL should handle cleanup
}

int catlm::load_utf8_text(std::string &file, std::vector<int> &dest, 
		   Vocabulary *vocab, bool add=true) {
  
  int wc = 0;
  
  std::ifstream inf;
  int id,i;
  std::string line;
  std::vector<std::string> tokens;
  int count = 0;
  inf.open(file.c_str());
  if (inf.is_open()) {
    while(!inf.eof()) {
      std::getline(inf, line);
      boost::split(tokens, line, boost::algorithm::is_any_of(" \t"));
      
      for (i = 0; i < tokens.size(); i++) {
	if (tokens[i].length()==0 || tokens[i][0] == ' ')
	  continue;
	id = vocab->get(tokens[i],add);
	if (id >= 0) {
	  count++;
	  dest.push_back(id);
	}
      }

      tokens.clear();
	    
    }
    inf.close();

  }
  
  return wc;
}


// Corpus class

Corpus::Corpus() {

}


Corpus::Corpus(string &path, bool isdir) {
  wc = 0;

  if (isdir) 
    load_from_directory(path);
  else 
    std::cerr << "Not yet implemented.\n";
  
}

Corpus::Corpus(string &path, Vocabulary &v, bool isdir) {

  wc = 0;

  vocab.add_all(v);

  if (isdir) 
    load_from_directory(path);
  else 
    std::cerr << "Not yet implemented.\n";
  
}


Corpus::~Corpus() {
  Document *d;
  while (docs.size() > 0) {
    d = docs.back();
    docs.pop_back();
    delete d;
  }

}

int Corpus::load_from_directory(string &path) {
  // list files
  int count=0;
  DIR *dir;
  struct dirent *ent;
  string fpath, fname, docid;
  std::size_t pos;

  if ((dir = opendir (path.c_str())) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      if ((ent->d_name)[0] == '.')
	continue;
      fname=string(ent->d_name);
      fpath = path + "/" + fname;
      pos = path.rfind('.');
      
      if (pos != std::string::npos) 
	add_document(fpath, docid = fname.substr(0,pos));
      else
	add_document(fpath, fname);
      count++;
    }
    closedir (dir);
  } else {
    std::cerr << "Unable to read directory entries from " << path;
    return 0;
  }
  
  return count;

}

bool Corpus::add_document(string &path, string &docid) {
  
  Document *d = new Document(path, &vocab, true);

  docs.push_back(d);
  docids.push_back(docid);
  docpaths.push_back(path);

  wc += d->size();
  oovs += d->oov_count();

  return true;
}
