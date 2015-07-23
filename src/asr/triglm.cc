 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "triglm.h"
#include "../utils/zlibutil.h"

using namespace catlm;
using namespace std;

#define CHUNK 32000

TriggerLM::TriggerLM(VocabTrie &trie, int _order, int _min_order, int maxtrig) : 
  vt(trie), order(_order), 
  p(trie.get_ngram_count()+trie.get_ngram_count(maxtrig)+1 ), min_order(_min_order) {
  p.setFixedA0(true);
  saveUtts=false;
  ngram_count = trie.get_ngram_count();
  trigger_count = trie.get_ngram_count(maxtrig);
}

TriggerLM::~TriggerLM() {

}

void TriggerLM::reset_counters() {
  lc=0;
}


int TriggerLM::read_nbest_feats(FILE *infile, gzFile infd, 
                                std::vector<std::map<int, double> > &uvecs, 
                                std::vector<double> &scores,
                                std::map<int, double> &hist,
				std::vector<std::vector<int> > *nbestlist) {
  
  //string key;
  int count = 0 ;
  char buf[CHUNK];
  char *ptr;
  int i;
  string key;

  scores.resize(0);
  uvecs.resize(0);

  if (lastkey.length() > 0) {
    key = current_key = lastkey;
  }
  else {
    if (!readLine(infile, infd, buf, CHUNK-1))
      return 0;

    lc++;

    ptr = strrchr(buf, '-');
    if (ptr) 
      *ptr = 0;
    else //malformed file
      return 0;
    current_key = key = buf;
    lastkey=key;
    
  }

  char *wd, *val;
  double am,lm, score;

  vector<string > words;
  map<int, double> vec;

  if (nbestlist)
    nbestlist->clear();


  if (saveUtts) {
    for(i=0; i < utts.size(); i++) {
      utts[i].wids.clear();
      utts[i].lens.clear();
      utts[i].scores.clear();
    }
    num_utts=0;
  }    

  vector<int> wids;
  vector<double> uscores;
  vector<int> lens;
  
  vector<string> unigram;
  unigram.push_back("");

  int id,len;

  while (!strcmp(key.c_str(), lastkey.c_str())) {
    wd = 0;
    words.resize(0);
    vec.clear();
    score = 0.0;
    wids.resize(0);
    if (nbestlist)
      nbestlist->push_back(wids);

    if (saveUtts) {
      uscores.resize(0);
      lens.resize(0);
    }

    // read the utterance info....
    while (readLine(infile, infd, buf, CHUNK-1)) {
      lc++;
      // expect this to be a word
      ptr = strchr(buf, ' ');
      if (!ptr) {
        fprintf(stderr, "Bad error at line %d: %s\n", lc, buf); break;
        // exit(1);
      }
               
      ptr = strchr(++ptr, ' ');
      if (!ptr) {
        //fprintf(stderr, "end state %s\n", buf);
        break;
      }
      wd = ptr+1;
      ptr = strchr(wd,' ');
      if (!ptr) {
        fprintf(stderr, "no WORD %s\n", buf);
        break;
      }
      *ptr = 0;
      //if (!strcmp(wd, "<eps>")) 
      //  continue; // skip epsilon
      
      val = ++ptr;
      ptr = strchr(ptr, ',');
      if (!ptr) {
        //fprintf(stderr, "no AM,LM score2 %s\n", buf);
        //words.push_back(wd);
        am = lm = 0;
        len = 0;
        //continue;
      }
      else {
        // parse AM, LM score then states
        *ptr = 0;
        
        am = strtod(val, 0);
        val = ++ptr;
        ptr = strchr(ptr, ',');
        if (!ptr) {
          fprintf(stderr, "No LM score....%s\n", buf);
          break;
        }
        *ptr = 0;
        lm = strtod(val, 0);
        //if (count == 73)
        //printf("NB %s-%d: %s %f %f %f\n", key.c_str(),count+1, wd, am, lm, score);
        ptr++;
        len = 0;
        while (ptr && *ptr) {
          ptr = strchr(ptr, '_');
          if (ptr) 
            ptr++;
          len++;
        }
        // fprintf(stderr, "%s %d\n", key.c_str(), len);

      }
      
      words.push_back(wd);

      unigram[0]=wd;
      id = vt.get_id(unigram, 0,1);
      if (id < 0) {
	unigram[0]="<unk>";
	id = vt.get_id(unigram, 0,1);
      }
      if (nbestlist)
	(*nbestlist)[count].push_back(id);

      if (saveUtts) {
        wids.push_back(id);
        lens.push_back(len);
        uscores.push_back(am+lm);
      }
      score += am+lm;

    } // parsed a line

    if (wd)
      count++;
    else {
      break; // no words read
    }
    
    if (saveUtts) {
      // keeping the utts vector, but resetting the others
      num_utts++;
      if (utts.size() < num_utts)
        utts.resize(num_utts);
      // utts[utts.size()-1] is all empty here
      
      for (i=0; i < wids.size();i++) {
        utts[num_utts-1].wids.push_back(wids[i]);
        utts[num_utts-1].lens.push_back(lens[i]);
        utts[num_utts-1].scores.push_back(uscores[i]);
      }

    }
      
    //    if (count == 75)
    //  fprintf(stderr, "score %s %f\n", key.c_str(), score);
        
    scores.push_back(score);


    int n,id,j;
    bool skip;
    // add all the 1-order in this utterance into the vector
    // offset by 1, since alpha[0] is fixed
    vec[0] = score;

    int maxb = 0;
    for (i=0; i < words.size(); i++) {
      if (!strcmp(words[i].c_str(), "<eps>"))  {
        //  skip=true; break; // don't add n-grams with the initial epsilon
        //continue;
      }
      for (n=1; n <= order; n++) {  // do 1 thru order grams
        if (i == words.size() - (n-1) )
          continue;
        // if order ==1 screw it
        if (n < min_order)
          continue;
        skip=false;
        for (j=0; j < n; j++) {
          if (i+j == words.size())
            break;
          if (!strcmp(words[i+j].c_str(), "<eps>"))  {
            skip=true; break; // don't add n-grams with the initial epsilon
          }
          else if (!strcmp(words[i+j].c_str(), "<silence>")) {
            skip=true; break;
          }
          else if (!strcmp(words[i+j].c_str(), "<noise>")) {
            skip=true; break;
          }
        }
       
        if (skip) continue;
        id = vt.get_id(words, i, n);
        //        if (n == 1 && id > maxb)
        //  maxb=id; // lazy
        if (vec.count(id+1) > 0) 
          vec[id+1] = vec[id+1]+1.0;
        else 
          vec[id+1] = 1.0;

      }
    }

    if (trigger_count > 0) {
      map<int,double>::iterator it;
      vector<int> adds;
      for (it=vec.begin(); it != vec.end(); it++) {
        if (it->second > 1 || hist.count(it->first) > 0)
          adds.push_back(it->first);
      }

      for (i=0; i < adds.size(); i++) {
        if (adds[i] < trigger_count)
          vec[adds[i]+ngram_count+1]=1;
      }
    }
    
    //print_vector(
    uvecs.push_back(vec);
    
    // go ahead and extract all N-grams in the utterance...

    if (!readLine(infile, infd, buf, CHUNK-1))
      break; // EOF

    lc++;    
    
    // might get EOF here, try to read a new key
    if (!readLine(infile, infd, buf, CHUNK-1)) {

      lc++;

      lastkey = "";
      //fprintf(stderr, "EOF %d\n", count);
      return count;
    }
    
    ptr = strrchr(buf, '-');
    if (ptr) 
      *ptr = 0; // strip off n-best counter
    else //malformed file
      return 0;
    key = buf;
    
  }
  

  lastkey = key;
  //fprintf(stderr, "next key is %s\n", lastkey.c_str());  

  return count;

}

NbestUtt* TriggerLM::get_utt(int i) {
  if (i >= 0 && i < utts.size())
    return &(utts[i]);
  else 
    return NULL;
}


//void TriggerLM::train_example(vector<int> &truth, 
int TriggerLM::train_example(map<int, double> &truth, 
                              vector<map<int, double> > &fvecs,
                              vector<double> &scores) {

  // 
  int i;
  int mini=0;
  double min = 9999;
  double score,err;


  double minerr = 9999999;
  int erri = 0;
  
  vector<double> errs; 
  for (i=0; i < scores.size(); i++) {
    score = p.score_example(fvecs[i]);
    scores[i]=score;
    //    err = p.l1Diff(fvecs[i], truth);
    //errs.push_back(err);
    //if (err < minerr) { 
    //  erri = i; minerr = err;
    //}
    if (score < min) { //+scores[i] < min) {
      min = score; //  + scores[i];
      mini=i;
    }

  }

  if (mini == erri) {
    //fprintf (stderr, "Found minerr %d %f\n", erri, minerr);
    //    print_vector(stderr, fvecs[mini] );
    // print_vector(stderr, truth );
    //    return mini;
  }
  if (mini != 0)  {
    //fprintf(stderr, "Selcted %s-%d %f/%f %f/%f\n", current_key.c_str(), 
    //        mini, score, scores[0], errs[mini], minerr); 
    //print_vector(stderr, fvecs[mini] );
    //print_vector(stderr, truth );
  }
  p.update_param(truth, fvecs[mini]);
//p.update_param(fvecs[erri], fvecs[mini]);

  // return mini;
  return erri;

}


int TriggerLM::rescore(vector<map<int, double> > &fvecs,
                      vector<double> &scores) {

  // 
  int i;
  int mini=0;
  double min = 999999;
  double score;


  for (i=0; i < scores.size(); i++) {
    score = p.score_example(fvecs[i]);
    if (score < min) { //  +scores[i] < min) {
      min = score; // + scores[i];
      mini=i;
    }
    scores[i] = score; //+= score;


  }

  //  mini=0;
  fprintf(stderr, "Selcted %s-%d %f %f\n", current_key.c_str(), mini,
          min, fvecs[mini][0]);


  return mini;
}

void catlm::print_vector(FILE* f, std::map<int, double> &vec) {
  map<int,double>::iterator it;
  fprintf(f, "{");
  for (it=vec.begin(); it != vec.end(); it++) {
    if (it == vec.begin())
      fprintf(f, "%d:%f", it->first, it->second);
    else
      fprintf(f, ", %d:%f", it->first, it->second);
    
  }
  fprintf(f, "}\n");
}

void catlm::add_vector(std::map<int, double> &dest, std::map<int, double> &src,
                       int maxid) {
  map<int,double>::iterator it;
  for (it=src.begin(); it != src.end(); it++) {
    if (it->first == 0)
      continue;
    if (maxid > 0 && it->first > maxid) 
      continue;
    if (dest.count(it->first) == 0)
      dest[it->first]=it->second;
    else
      dest[it->first]+=it->second;
  }

}


void TriggerLM::debug() {
  int i=0;
  fprintf(stderr, "alpha[ASR]=%f\n", p.get_alpha(0));
  for (i=0; i < vt.get_ngram_count(); i++) {
    if (p.get_alpha(i+1) > 0) {
      fprintf(stderr, "alpha[%d,%s]=%f\n", i+1,vt.get_word(i).c_str(), p.get_alpha(i+1));
    }
  }
}

void TriggerLM::write(const char* file, bool writengrams) {

  // need to write out
  FILE* fp = fopen(file, "wb");
  if (!fp)
    return;

  int x = TRIGLM_VER;
  fwrite("#TRIGLM", 1, 7, fp);
  fwrite(&x, sizeof(int), 1, fp);
  
  
  if (writengrams) {
    x=1;
    fwrite(&x, sizeof(int), 1, fp);
  }
  else {
    x=0;
    fwrite(&x, sizeof(int), 1, fp);
  }
  
  p.write(fp);

  fclose(fp);

}

bool TriggerLM::read(const char* file, const char *lm) {
  int readb;
  // need to write out
  fprintf(stderr, "reading model from %s\n", file);
  FILE* fp = fopen(file, "rb");
  char buf[1000];
  memset(buf, 0, 1000);
  readb = fread(buf, 1, 7, fp);
  if (readb != 7 || strcmp(buf, "#TRIGLM")) {
    fclose(fp);
    fprintf(stderr, "invalid format\n");
    return false;
  }
  
  int x;
  readb = fread(&x, sizeof(int), 1, fp);
  if (readb != 1) {
    fprintf(stderr, "invalid format - ver\n");
    fclose(fp);
    return false;
  }
    
  readb = fread(&x, sizeof(int), 1, fp);
  if (readb != 1) {
    fprintf(stderr, "invalid format - lm\n");
    fclose(fp);
    return false;
  }

  if (x) {
    // read ngrams
  }
  else {
    if (lm) {
      vt.load(lm);
    }
  }


  fprintf(stderr, "Reading perceptron weights...\n");
  if (p.read(fp)) {
    fclose(fp);
    return false;
  }
  else {
    fclose(fp);
    return false;
  }

}

void TriggerLM::reset() {
  lastkey="";
  current_key="";
}
