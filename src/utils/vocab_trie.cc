#include "vocab_trie.h"
#include "zlibutil.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace catlm;

using std::string;

VocabNode::VocabNode(int vid, VocabNode *vparent) : id (vid), parent (vparent) {
  // nothign to be done?
}

VocabNode::~VocabNode() {
  std::map<string,VocabNode*>::iterator it;
  for (it = children.begin(); it != children.end(); it++) {
    if (it->second)
      delete it->second;
  }
}


VocabTrie::VocabTrie() {
  root = new VocabNode(-1, (VocabNode*)NULL);
  maxid = -1;
}

VocabTrie::~VocabTrie() {
  if (root)
    delete root;
}


int VocabTrie::insert(std::vector<const char*> &words) {
  int pos;
  VocabNode *node = root;
  for (pos=0; pos < words.size(); pos++) {
    node = insert(node, words, pos);
  }
  return node->id;
}


VocabNode *VocabTrie::insert(VocabNode *here, std::vector<const char*> &words, int idx) {
  std::map<string,VocabNode*>::iterator it;
  it = here->children.find(words[idx]);

  if (it != here->children.end()) // word already in trie
    return it->second;
  
  VocabNode *node = new VocabNode(++maxid, here);
  string w(words[idx]);
  here->children[w]=node;
  return node;
}


int VocabTrie::get_id(std::vector<const char*> &words) {
  VocabNode *node = root;
  int pos = 0;
  std::map<string,VocabNode*>::iterator it;
  string w;

  while (node && pos < (int) words.size()) {
    w = words[pos];    
    it = node->children.find(w);
    if (it == node->children.end())
      break; // not found

    node = it->second;
    pos++;
      
  }
  if (pos == (int) words.size()) 
    return node->id;
  
  return -1;
  

}

int VocabTrie::load(const char* file) {
  char line[0x10000];
  int bsize = 0x10000;
  char *sep, *sep2;
  fprintf(stderr, "Opening %s %d for reading\n", file, optind);

  FILE *infile=fopen(file, "r");
  int len = strlen(file);
  bool gzipped=(len > 3 && strncmp(file+len-3, ".gz",3)==0);

  if(gzipped) 
    init_gzip_stream(infile,&line[0]);

  std::vector<int> ngc(3);
  int ng = 0;

  int count,pos;

  std::vector<const char*> ngram;

  while (readLine(infile,line,gzipped,bsize)) {
    // chec
    len = strlen(line);
    if (len == 0)
      continue;

    //      printf("%s\n",line);

    //    if (1) continue;

    if (!strncmp(line, "ngram ", 6)) {
      sep=strchr(line, '=');
      if (sep) {
	*sep=0;
	ng = atoi(line+6);
	if (ng > ngc.size())
	  ngc.resize(ng, 0);
	if (sep - line < len) {
	  ngc[ng-1]=atoi(sep+1);
	  
	  fprintf(stderr, "Ngram count %d %d\n", ng, ngc[ng-1]);
	}
      }
      ng = 0;
    }
    else if (line[0] == '\\' && len > 3) {
      sep = strchr(line, '-');
      if (sep) {
	*sep=0;
	if (ng > 0) 
	  printf("processed %d %d-grams\n", count, ng);
	ng = atoi(line+1);
	
	printf("processing %d-grams\n", ng);
	count = 0;
      }
    }
    else {
      if (ng > 0) { // parse an n-gram
	sep = strchr(line, '\t');
	
	if (sep && len > (sep-line)) {
	  *sep=0;
	  sep++;
	  
	  sep2 = strchr(sep, '\t');
	  if (sep2) {
	    *sep2=0;
	  }
	  
	  count++;
	  pos = 0;
	  sep2=sep;
	  //	  if (ng == 1) continue;
	  //printf("NG: ", sep);  
	  if (ngram.size() < ng)
	    ngram.resize(ng, 0);
	  while (sep[0] != 0 && (sep2 = strchr(sep, ' ')) != NULL) {
	    *sep2=0;
	    //printf("%s ", sep);  
	    ngram[pos]=sep;
	    sep=sep2+1;
	    pos++;
	  }
	  ngram[pos]=sep;

	  pos = insert(ngram);
	  //printf("%d", pos);
	  //for (pos=0; pos < ngram.size();pos++) {
	  //  printf(" %s", ngram[pos]);
	  //}

	  //printf("\n");
	  
	}
      }
    }
  }

  fclose(infile);

  if (ng > 0) 
    printf("processed %d %d-grams\n", count, ng);

  return maxid+1;

}

void print_ngram(std::vector<const char*> ng) {
  int i=0;
  printf("[");
  for (i=0; i < ng.size(); i++) {
    if (i!= 0)
      printf("-%s",ng[i]);
    else
      printf("%s",ng[i]);
  }
  printf("]");

}

void VocabTrie::extract_vec(const char* text, std::map<int, double> &vec, int maxorder) {
  
  std::vector<const char *> words;
  std::vector<const char *> ngram;
  int x = 0;
  char *buf = new char[strlen(text)+1];
  strcpy(buf, text);
  char *ptr=buf;
  char *ptr2=0;
  ptr2 = strchr(ptr, ' ');
  //    printf("%s\n", ptr);
  while (ptr2) {
    *ptr2 = 0;
    words.push_back(ptr);
    ptr=ptr2+1;
    if (*ptr == 0)
      break;
    ptr2= strchr(ptr, ' ');
  }
  words.push_back(ptr);

  int n=0; int j,i,id;
  for (n=1; n <= maxorder; n++) {
    ngram.resize(n, 0);
    //    printf("%d %d\n", n, words.size()-n);
    for (j=0; j <= ((int) words.size()-n); j++) {
      int x= words.size();
      //printf("%d %d word %s\n", j, words.size()-n, words[j]);
      for (i=0; i < n; i++) {
	ngram[i]=words[j+i];
      }
      id = get_id(ngram);
      
      if (id >= 0) {
	vec[i]=1.0;
	/*printf("found%d ", ngram.size());
	print_ngram(ngram);
	printf(": %d\n", id);
	*/
      }
      else {
	//printf("%d ",id); print_ngram(ngram), printf("\n");
      }
    }
    // fill in N-gram, lookup ID, set vector;
  }

  delete [] buf;
  
}
