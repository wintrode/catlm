#include "perceptron.h"

#include <string.h>
#include <stdio.h>

using namespace catlm;

using std::string;

Perceptron::Perceptron(int dim, double eps) : fmax(dim-1), fixedA0(false) {
  // nothign to be done?
  epsilon=eps;
  alpha=new double[dim];
  amean = new double[dim];
  N=0;
  memset(alpha, 0, sizeof(double)*dim);
  memset(amean, 0, sizeof(double)*dim);
}


Perceptron::~Perceptron() {
  // nothign to be done?
  delete [] alpha;
  delete [] amean;
}

double Perceptron::score_example(fvec &hyp) {
  double score = 0.0;
  fiterator it;
  for (it = hyp.begin(); it != hyp.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      //fprintf(stderr, "score: %d %f %f\n", it->first, it->second, alpha[it->first]);
      score += it->second * alpha[it->first];
      
    }
  }
  return score;
}


void Perceptron::update_param(fvec &truth, fvec &hyp, int minid) {
  fiterator it;
  double adiff;

  for (it = hyp.begin(); it != hyp.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      if (truth.count(it->first) > 0)
        adiff = it->second - truth[it->first];
      else
        adiff = it->second;

      if (fixedA0 && it->first == 0)
        continue;
      
      if (it->first < minid)
        continue;


      alpha[it->first] += adiff;
    }
  }

  for (it = truth.begin(); it != truth.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      if (hyp.count(it->first) > 0)
        continue;
      adiff = -truth[it->first];

      if (fixedA0 && it->first == 0)
        continue;

      //amean[it->first] += alpha[it->first] + adiff;
      if (it->first < minid)
        continue;

      alpha[it->first] += adiff;

      // ehhe 
      if (it->first == 23929)  {
        fprintf (stderr, "alpha[%d]=%f\n", it->first, alpha[it->first]);
      }

    }

  }

  int i=0;
  for (i=0; i <= fmax;i++)
    amean[i] += alpha[i]; 
  N++;

}


void Perceptron::update_param(double truth, fvec &hyp, int minid) {
  fiterator it;
  double adiff;

  double score = 0;
  for (it = hyp.begin(); it != hyp.end(); it++) {

    //adiff = epsilon * alpha[it->first] * it->second - truth;
    score += alpha[it->first] * it->second;
    adiff = epsilon * it->second * truth;
    
    if (fixedA0 && it->first == 0)
      continue;
    
    if (it->first < minid)
      continue;
    
    alpha[it->first] += adiff;
    //if (truth==1.0)
    //  fprintf(stderr, "ADIFF %f %f %f %d %d\n", truth, it->second, alpha[it->first], it->first, fmax);
  }

  if (score > 0 && truth == 1.0)
    ccorrect++;
  else if (score < 0 && truth == -1.0)
    ccorrect++;

  tclass++;

  
  int i=0;
  for (i=0; i <= fmax;i++)
    amean[i] += alpha[i]; 
  N++;

}





void Perceptron::update_param(fvec &delta, int minid) {
  fiterator it;
  double adiff;

  for (it = delta.begin(); it != delta.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {

      if (fixedA0 && it->first == 0)
        continue;

      if (it->first < minid)
        continue;

      alpha[it->first] += it->second;
    }
  }

  int i=0;
  for (i=0; i <= fmax;i++)
    amean[i] += alpha[i]; 
  N++;

}


void Perceptron::write(FILE *fp) {
  int x = fmax+1;
  fwrite(&x, sizeof(int), 1, fp);
  fwrite(alpha, sizeof(double), x, fp);
  fwrite(amean, sizeof(double), x, fp);
  fwrite(&N, sizeof(int), 1, fp);
  
}

bool Perceptron::read(FILE *fp) {
  int x;
  int readb = fread(&x, sizeof(int), 1, fp);
  if (readb != 1) {
    fclose(fp);
    return false;
  }
  
  if (alpha)
    delete [] alpha;
  
  if (amean)
    delete [] amean;
  
  alpha= new double[x];
  amean = new double[x];

  fmax = x-1;
  readb = fread(alpha, sizeof(double), x, fp);
  if (readb != x) {
    fclose(fp);
    return false;
  }
  
  readb = fread(amean, sizeof(double), x, fp);
  if (readb != x) {
    fclose(fp);
    return false;
  }

   
  readb = fread(&x, sizeof(int), 1, fp);
  if (readb != 1) {
    fclose(fp);
    return false;
  }

  N=x; // normalize amean?

  double olda0=alpha[0];



  // average alpha 
  //alpha[0]=10;
  //alpha[0]=olda0;
  if(N > 0) {
    memcpy(alpha, amean, sizeof(double)* fmax);
    for (x=0; x < fmax; x++) {
      fprintf(stderr, "alpha[%d]=%f\n", x, alpha[x] / N);
      alpha[x] /= (double) N;
    }
  }
  


  return true;
    

}

double Perceptron::score_example(fvec &hyp, fvec &dif) {
  double score = 0.0;
  fiterator it;

  for (it = hyp.begin(); it != hyp.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      if (dif.count(it->first) > 0)
        score += (it->second - dif[it->first]) * alpha[it->first];
      else
        score += it->second * alpha[it->first];
    }
  }
  for (it = dif.begin(); it != dif.end(); it++) {
    if (it->first >= 0 && it->first <= fmax && hyp.count(it->first) == 0) 
      score += it->second * -alpha[it->first];
  }
  return score;
}
