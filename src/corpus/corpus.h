// corpus/corpus.h

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

#ifndef CATLM_CORPUS_H
#define CATLM_CORPUS_H

#include <string>
#include <map>
#include <vector>
#include <iostream>


namespace catlm {

  using std::string;

  class Vocabulary {
    
  private:

    std::vector<string> vocab;
    std::map<string,int> idmap;

    string unk;
    int unk_id;

  public:
    Vocabulary();
    Vocabulary(string &file);
    ~Vocabulary();

    int size() {
      return idmap.size();
    }

    int set_oov_token(string &oov) {
      unk=oov;
      unk_id=get(unk, true);
    }

    int add_all(Vocabulary &v) {
      int i;
      for (i=0; i < v.vocab.size(); i++) {
	vocab.push_back(v.vocab[i]);
	idmap[v.vocab[i]]=i;
      }
      unk = v.unk;
      unk_id = v.unk_id;
    }

    // should be synchronized
    int get(string &wtype, bool add=true) {
      std::map<string,int>::iterator it;
      it = idmap.find(wtype);

      if (it == idmap.end()) {
	if (add) {
	  idmap[wtype]=this->vocab.size();
	  vocab.push_back(wtype);
	  return this->vocab.size();
	}
	else {
	  return unk_id;
	}
      }
      else {
	return it->second;
      }
	  
    }
    
    string *get(int id) {
      if (id < 0 || id >= vocab.size())
	return NULL;
      return &(vocab[id]);
    }

    void load(string& filename);
    void save(string& filename);

  };


  class Document {

  private:

    std::vector<int> tokens;
    int oovs;

  public:
    Document();
    Document(std::vector<int> &tokens);
    Document(std::string &file, Vocabulary *vocab, bool add);
    ~Document();

    int get_token(int position) {
      if (position >= 0 && position < tokens.size())
	return tokens[position];
      else
	return -1;
    }

    int size() {
      return tokens.size();
    }

    int oov_count() {
      return oovs;
    }
    
    

  };


  class SegmentedDocument: public Document {

  private:

    std::vector<int> segments;
    
  public:
    SegmentedDocument();
    ~SegmentedDocument();
    
    int sgement_count() {
      return segments.size();
    }

  };



  class Corpus {

  private:
    std::vector<Document*> docs;
    std::vector<string> docids;
    std::vector<string> docpaths;

    std::map<string,int> idmap;
    
    Vocabulary vocab;

    int wc;
    int oovs;

  public:
    Corpus();
    Corpus(string &path, bool isdir=true);
    Corpus(string &path, Vocabulary &v, bool isdir=true);
    ~Corpus();

    Vocabulary *vocabulary() {
      return &vocab;
    }
    
    Document *doc_at(int position) {
      if (position >= 0 && position < docs.size())
	return docs[position];
      else
	return NULL;
    }

    Document *doc_for(string&  docid) {
      std::map<string,int>::iterator it = idmap.find(docid);
      if (it == idmap.end())
	return NULL;
      
      return docs[it->second];
    }

    int load_from_directory(string &path);
    bool add_document(string &path, string &docid);

    void debug_stats() {
      std::cerr << "Documents " << docs.size() << "\n" <<
	"Words " << wc << "\n";
      std::cerr << "OOVs " << oovs << "\n";
    }

    int oov_count() {
      return oovs;
    }


  };



  int load_utf8_text(std::string &file, std::vector<int> &dest, 
		     Vocabulary *vocab, bool add);
}

#endif

