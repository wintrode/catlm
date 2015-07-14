// utils/utils.cc

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

#include "utils.h"

using namespace std;
  
int read_set(string &filename, set<int>& strset, int(*idlookup)(string&)) {
    ifstream inf;
    int id;
    int count = 0;
    string word;

    inf.open(filename.c_str());
    if (inf.is_open()) {
      while(!inf.eof()) { 
	getline(inf, word);
	if (inf.eof()) break;
	if ( word.size() < 1 ) continue;
	
	id = (*idlookup)(word);
	strset.insert(id);
	count++;
      }
      inf.close();
    }
    
    return count;
    
  }
  
  int read_map(string &filename, map<int, double>& strmap, int(*idlookup)(string&)) {
    ifstream inf;
    int id;
    int count = 0;
    string word;
    double val;
    inf.open(filename.c_str());
    if (inf.is_open()) {
      while(!inf.eof()) { 
	inf >> word;
	if ( word.size() < 1 ) continue;
	if (inf.eof()) break;
	inf >> val;
	if (inf.eof()) break;
	
	
	id = (*idlookup)(word);
	strmap[id] = val;
	count++;
      }
      inf.close();
    }
    
    return count;
    
  }
  
  
  int read_map(std::string &filename, std::map<std::string, int>& strmap, 
	       std::vector<std::string> *idlist) {
    ifstream inf;
    int id = -1;
    int max = 0;
    int count = 0;
    string word;
    int val;
    map<string,int>::iterator it;
    inf.open(filename.c_str());
    if (inf.is_open()) {
	while(!inf.eof()) { 
	    inf >> val;
	    if (inf.eof()) break;
	    inf >> word;
	    if (inf.eof()) break;
	    if ( word.size() < 1 ) continue;
	    
	    if (val >= 0)
	      strmap[word]=val;
	    else
	      continue;
	    if (idlist) {
	      if (val >= idlist->size())
		idlist->resize(val+1, string(""));
	      (*idlist)[val]=word;
	    }
	    
	    
	    count++;
	}
	inf.close();
    }

    return count;

}


  int read_counts(string &filename, map<int, double>& strmap, int(*idlookup)(string&)) {
    ifstream inf;
    int id;
    int count = 0;
    string word;
    double val;
    inf.open(filename.c_str());
    if (inf.is_open()) {
	while(!inf.eof()) { 
	    inf >> word;
	    if ( word.size() < 1 ) continue;
	    if (inf.eof()) break;
	    inf >> val;
	    if (inf.eof()) break;

	    id = (*idlookup)(word);
	    if (strmap.find(id) == strmap.end()) 
		strmap[id] = 0.0;
	    strmap[id] += val;
	    count++;
	}
	inf.close();
    }
    return strmap.size();
}
  
int write_map(std::string &filename, std::map<int, double>& strmap,
	      std::vector<string>& vocab) {

    FILE *fp;
    //open FILE, ">:utf8", $fweights;
    fp = fopen(filename.c_str(), "w");
    map<int,double>::iterator si;
    int count = 0;
    if (fp) {

	for (si=strmap.begin(); si != strmap.end(); si++) {
	    if (si->first < vocab.size()) {
		fprintf(fp, "%s %g\n", vocab[(*si).first].c_str(), (*si).second);
		count++;
	    }
	}
		
	fclose(fp);
	
    }
    
    return count;
    
}


int write_map(std::string &filename, std::map<std::string, int>& strmap) {

    FILE *fp;
    //open FILE, ">:utf8", $fweights;
    fp = fopen(filename.c_str(), "w");
    map<string,int>::iterator si;
    int count = 0;
    if (fp) {

	for (si=strmap.begin(); si != strmap.end(); si++) {
	  fprintf(fp, "%d %s\n", (*si).second,  (*si).first.c_str());
	  count++;
	}
		
	fclose(fp);
	
    }
    
    return count;
    
}

int write_vector(std::string &filename, std::vector<std::string>& list) {

    FILE *fp;
    //open FILE, ">:utf8", $fweights;
    fp = fopen(filename.c_str(), "w");
    int count = 0;
    if (fp) {
      int i=0;
      for (i=0; i < list.size(); i++) {
	fprintf(fp, "%d %s\n", i, list[i].c_str());
	count++;
      }
      
      fclose(fp);
	
    }
    
    return count;
    
}


int split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    elems.clear();
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems.size();
}

