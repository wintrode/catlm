#include "perceptron.h"

#include <string.h>
#include <stdio.h>

using namespace catlm;

using std::string;

Perceptron::Perceptron(int dim) : fmax(dim-1) {
  // nothign to be done?
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

void Perceptron::update_param(fvec &truth, fvec &hyp) {
  fiterator it;
  double adiff;

  for (it = hyp.begin(); it != hyp.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      if (truth.count(it->first) > 0)
        adiff = it->second - truth[it->first];
      else
        adiff = it->second;

      amean[it->first] += alpha[it->first] + adiff;
      alpha[it->first] += adiff;
    }
  }

  for (it = truth.begin(); it != truth.end(); it++) {
    if (it->first >= 0 && it->first <= fmax) {
      if (hyp.count(it->first) > 0)
        continue;
      adiff = -truth[it->first];
      amean[it->first] += alpha[it->first] + adiff;
      alpha[it->first] += adiff;

    }

  }
  
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
  if (readb != sizeof(int)) {
    fclose(fp);
    return false;
  }
  
  if (alpha)
    delete [] alpha;
  
  if (amean)
    delete [] amean;
  
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
  if (readb != sizeof(int)) {
    fclose(fp);
    return false;
  }

  N=x; // normalize amean?
  
  return true;
    

}
