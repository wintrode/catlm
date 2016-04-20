 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <list>

#include "triglm.h"
#include "../utils/zlibutil.h"

using namespace catlm;
using namespace std;

#define CHUNK 32000

TriggerLM::TriggerLM(VocabTrie &trie, int _order, int _min_order, int maxtrig, bool fullng) : 
  vt(trie), order(_order), 
  p(fullng ? (trie.get_ngram_count(1)*_order + trie.get_ngram_count(1)*maxtrig+1) :
    trie.get_ngram_count()+trie.get_ngram_count(maxtrig)+1 ), min_order(_min_order) {
  p.setFixedA0(true);
  saveUtts=false;
  ngram_count = trie.get_ngram_count();
  trigger_count = trie.get_ngram_count(maxtrig);
  lambda = 1.0;
  trigOnly=false;
  vsize=vt.get_ngram_count(1);
}

TriggerLM::~TriggerLM() {

}

void TriggerLM::reset_counters() {
  lc=0;
  p.reset_status();
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
        if (adds[i] < trigger_count) {
          //if (adds[i] == 2758) {
          //  fprintf(stderr, "Adding ehhe %d to history in %s\n", adds[i]+ngram_count, key.c_str());
          //}
          vec[adds[i]+ngram_count]=1;
        }
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




int TriggerLM::read_lattice_feats(FILE *infile, gzFile infd, 
                                std::vector<std::map<int, double> > &uvecs, 
                                std::vector<latarc> &arcs,
                                  std::map<int, double> &hist,
                                  std::map<int, std::string> *vmap) {
  
  //string key;
  int count = 0 ;
  char buf[CHUNK];
  char *ptr;
  int i;
  string key;

  uvecs.resize(0);

  if (!readLine(infile, infd, buf, CHUNK-1))
    return 0;

  lc++;

  ptr = strchr(buf, ' ');
  if (ptr) 
    *ptr=0;

  ptr = strchr(buf, '\t');
  if (ptr) 
    *ptr=0;
  
  
  current_key = key = buf;
  lastkey=key;
    
  char *wd, *val, *estate;
  double am,lm, score;

  vector<string > words;
  map<int, double> vec;

  vector<int> wids;
  vector<double> uscores;
  vector<int> lens;
  
  vector<string> unigram;
  unigram.push_back("");

  int id,len,wid;
  int arcstart, arcend;

  wd = 0;
  words.resize(0);
  vec.clear();
  score = 0.0;
  wids.resize(0);
  latarc arc;
  vector< vector<latarc> > lattmp;
  vector< vector<latarc> > lattmp2;

  int maxend = 0;

  // read the utterance info....
  while (readLine(infile, infd, buf, CHUNK-1)) {
      lc++;

      if (strlen(buf) == 0)
        break;
      //fprintf(stderr, "READING LINE %d: %s\n", lc, buf);

      // expect this to be a word
      estate = ptr = strchr(buf, '\t');
      if (!ptr) {
        //fprintf(stderr, "Bad error at line %d: %s\n", lc, buf); continue;
        continue;
        // exit(1);
      }
      
      *ptr=0;
      arcstart = atoi(buf);
               
      ptr = strchr(++ptr, '\t');
      if (!ptr) {
       
        ptr = strchr(estate, ',');
        if (!ptr) {
          //fprintf(stderr, "end state %s\n", buf);
          continue;
        }
        // final state....
        continue;
      }
      *ptr=0;
      arcend=atoi(estate+1);

      wd = ptr+1;
      ptr = strchr(wd,'\t');
      if (!ptr) {
        //fprintf(stderr, "no WORD %s\n", buf);
        //break;
        am = lm = 0;
        len = 0;
      }
      else { 
        *ptr = 0;
        

        if (vmap) {
          wid = atoi(wd);
          if (vmap->count(wid) > 0)
            unigram[0]=(*vmap)[wid];
          else unigram[0] = "<eps>";
        }
        else {
          unigram[0]=wd;
        }
        wid = vt.get_id(unigram, 0,1);

        //wid = atoi(wd);
        // word or word id? 
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
      }

      //fprintf(stderr, " Found WID %s %d,%d %f %f\n", wd, arcstart, arcend, am, lm);
      
      words.push_back(wd);

      unigram[0]=wd;
      id = vt.get_id(unigram, 0,1);
      if (id < 0) {
	unigram[0]="<unk>";
	id = vt.get_id(unigram, 0,1);
      }

      // now we've got a lattice arc
      // want one training example per arc
      // The feature vector for the arc should be
      // a:  the word's AM+LM score
      // b:  the preceeding words AM+LM score
      // (c):  the n-2 word's AM+LM score
      arc.start = arcstart;
      arc.end = arcend;
      arc.wid = wid;
      arc.cost = am + lm;
      arc.len = len;

      if (lattmp.size() <= arc.end) {
        lattmp.resize(arc.end+1);
      }
      lattmp[arc.end].push_back(arc);

      if (lattmp2.size() <= arc.start) {
        lattmp2.resize(arc.start+1);
      }
      lattmp2[arc.start].push_back(arc);

      if (arc.end > maxend)
        maxend = arc.end;

      // parsed a line

      if (wd)
        count++;
      else {
        continue; // no words read
      }
      
      
      // go ahead and extract all N-grams in the utterance...
      
  }
  
  fprintf(stderr, "Done parsing %s\n", key.c_str());
  
  std::list<int> list;
  std::list<int> newlist;

  int j,k,n ;

  std::vector<int> time;
  time.resize(maxend+1, 0);
  time[0] = 0;
  for (i=0; i < lattmp2.size(); i++) {
    for (j=0; j < lattmp2[i].size(); j++) {
      if (time[lattmp2[i][j].end] == 0 && lattmp2[i][j].len > 0)
        time[lattmp2[i][j].end] = time[lattmp2[i][j].start] + lattmp2[i][j].len;
    }
  }

  
  

  // okay, loop over the arcs, indexed by end state
  for (i=0; i < lattmp.size(); i++) {
    for (j=0; j < lattmp[i].size(); j++) {
      // unigram features
      vec.clear();
      arc = lattmp[i][j];
      vec[arc.wid+1]=arc.cost;
      
      //fprintf(stderr, "ARC: (%d,%d):(%d,%d) %d=%f\n", time[arc.start],time[arc.end],arc.start, arc.end,arc.wid, arc.cost);
      // look back to arc.start
      arcend = arc.start;
      list.clear();
      list.push_back(arcend);
      
      for (n=1; n < order; n++) {
        newlist.clear();
        
        while (!list.empty()) {
          arcend  = list.front();
          list.pop_front();

          for (k=0; k < lattmp[arcend].size(); k++) {
            arc = lattmp[arcend][k];
            //fprintf(stderr, "adding %d %d %f\n", n, n*vsize+arc.wid+1, arc.cost);
            vec[n*vsize + arc.wid + 1] = arc.cost;
            if (n + 1 < order) // don't bother filling newlist if the loop will end
              newlist.push_back(arc.start);
          }
        }
        list = newlist;
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
      
      uvecs.push_back(vec);

      arc = lattmp[i][j];
      arc.start = time[arc.start];
      arc.end = time[arc.end];
      arcs.push_back(arc);

    }
  }

  
  return count;

}

NbestUtt* TriggerLM::get_utt(int i) {
  if (i >= 0 && i < utts.size())
    return &(utts[i]);
  else 
    return NULL;
}


//void TriggerLM::train_example(vector<int> &truth, 
/*int TriggerLM::train_example(map<int, double> &truth, 
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

  if (trigOnly)
    p.update_param(truth, fvecs[mini], ngram_count+1);
  else
    p.update_param(truth, fvecs[mini]);
//p.update_param(fvecs[erri], fvecs[mini]);

  // return mini;
  return erri;

  }*/

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

  if (trigOnly)
    p.update_param(truth, fvecs[mini], ngram_count+1);
  else
    p.update_param(truth, fvecs[mini]);
//p.update_param(fvecs[erri], fvecs[mini]);

  // return mini;
  return erri;

}

int TriggerLM::train_example_lattice(std::vector<std::map<int, double> > &fvecs,
                                     std::vector<double> errs)  {

  int i;
  for (i=0; i < errs.size(); i++) {
   
    if (trigOnly)
      p.update_param(errs[i], fvecs[i], ngram_count+1);
    else 
      p.update_param(errs[i], fvecs[i]);
    
 
  }

  return 0;

}



//void TriggerLM::train_example(vector<int> &truth, 
int TriggerLM::train_example_soft(vector<map<int, double> > &fvecs,
                                  vector<double> errs, 
                                  vector<double> &scores) {

  // 
  int i;
  int mini=0;
  double min = 9999;
  double score,err;


  double minerr = 9999999;
  int erri = 0;
  
  vector<int> C;
  vector<int> E;
  int mincost = fvecs[0].size();
  for (i=0; i < errs.size(); i++) {
    if (errs[i] < mincost)
      mincost = errs[i];
  }
  for (i=0; i < errs.size(); i++) {
    errs[i] -= mincost;
  }

  int j;
  double d;
  
  for (i=0; i < fvecs.size(); i++) {
    if (errs[i] == 0.0) { // epsilon?
      // candidate for C
      for (j = 0; j < fvecs.size(); j++) {
        if (errs[j] == 0.0)
          continue;
        d=p.score_example(fvecs[i], fvecs[j]);
        if (d < lambda * errs[j]) {
          C.push_back(i);
          break;
        }
      }
    }
    else {
      // candidate for E
      for (j = 0; j < fvecs.size(); j++) {
        if (errs[j] > 0.0)
          continue;
        d=p.score_example(fvecs[j], fvecs[i]);
        if (d < lambda * errs[i]) {
          E.push_back(i);
          break;
        }
        
      }
    }
  }
  
  if (C.size() == 0) //  || E.size() == 0)
    return 0; 

  fvec delta;
  double denom = 0.0;
  for (i=0; i < C.size(); i++) {
      d = 1.0 / (double)C.size(); // tau(c)       
      add_vector(delta, fvecs[C[i]], d);
  }
  denom = 0.0;
  vector<double> te;

  for (i=0; i < E.size(); i++) {
    d = 0.0;
    for (j=0; j < C.size(); j++) {
      if (p.score_example(fvecs[C[j]], fvecs[E[i]]) > lambda * errs[E[i]]) {
        d += 1.0;
        denom += C.size();
      }
    }
    te.push_back(d);
  }
  
  for (i=0; i < E.size(); i++) {
    add_vector(delta, fvecs[E[i]], -te[i]/denom);
  }
  if (trigOnly)
    p.update_param(delta, ngram_count+1);
  else
    p.update_param(delta);

  return 0;

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


double TriggerLM::rescore_lattice(map<int, double> &fvec) {

  return p.score_example(fvec);
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

void catlm::add_vector(std::map<int, double> &dest, std::map<int, double> &src,
                       double scalar) {
  map<int,double>::iterator it;
  for (it=src.begin(); it != src.end(); it++) {
    if (it->first == 0)
      continue;

    if (dest.count(it->first) == 0)
      dest[it->first]=scalar * it->second;
    else
      dest[it->first]+= scalar * it->second;
  }

}


void TriggerLM::debug() {
  int i=0;
  fprintf(stderr, "alpha[ASR]=%f\n", p.get_alpha(0));
  for (i=0; i < p.get_fmax(); i++) {
    if (p.get_alpha(i+1) != 0) {
      if (i <= ngram_count)
        fprintf(stderr, "alpha[%d,%s]=%f\n", i+1,vt.get_word(i).c_str(), p.get_alpha(i+1));
      else
        fprintf(stderr, "alpha[TRIGGER(%d),%s]=%f\n", i+1,vt.get_word(i-ngram_count).c_str(), p.get_alpha(i+1));
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
