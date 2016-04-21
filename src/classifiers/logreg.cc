#include "logreg.h"

#include <string.h>
#include <stdio.h>

#include <vector>
#include <fstream>

#include <stdint.h>
#include <stdlib.h>



using namespace catlm;

using namespace std;

LogReg::LogReg(int dim, double eps) : fdim(dim), rate(eps) {
  // nothign to be done?
  //epsilon=eps;
  alpha=new double[dim+1];
  //N=0;
  //memset(alpha, 0, sizeof(double)*dim+1);
  int i;
  for (i=0; i <= dim; i++)
    alpha[i] = drand48()*2 - 1;
  
}


LogReg::~LogReg() {
  // nothign to be done?
  delete [] alpha;

}

double LogReg::predict(fvec &hyp) {
  double score = 0.0;
  fiterator it;
  for (it = hyp.begin(); it != hyp.end(); it++) {
    if (it->first >= 0 && it->first <= fdim) {
      //fprintf(stderr, "score: %d %f %f\n", it->first, it->second, alpha[it->first]);
      score += it->second * alpha[it->first];
      
    }
  }
  return 1.0 / exp(-score);
}

double LogReg::compute_gradient(fvec &grad, vector<fvec> &vecs, vector<int> &labels) {
  int i,j;
  double h;
  for (j=0; j <= fdim; j++) {
    grad[j]=0;
  }

  double lambda = 1;
  vector<double> gtest;
  gtest.resize(fdim+1, 0.0);

  vector<double> ltest;
  ltest.resize(fdim+1, 0.0);

  double loss = 0.0;
  double lg = 0.0;
  double p;
  for (i=0; i < labels.size(); i++) {
    p = predict(vecs[i]);
    if (labels[i] == 1)
      fprintf(stderr, "h(x[%d]) = %f\n", i, h);
    loss -= (labels[i] * log(p) + (1-labels[i]) * log(1-p));
    
    

    h = p - (double) labels[i];
    for (j=0; j <= fdim; j++) {
      if (vecs[i].find(j) != vecs[i].end()) {
	grad[j] += vecs[i][j] * h;
	gtest[j] +=  vecs[i][j] * h;
	//ltest[j] += (labels[i] * log(p)
      }
    }
  }
  loss /= (double) labels.size();
  return loss;

}

void LogReg::train(int iterations, vector<fvec> &vecs, vector<int> &labels) {
  fprintf(stderr, "Training\n");
  int i,j;
  double loss;
  for (i=0; i < iterations; i++) {

    //if (i % 100 == 0) fprintf(stderr, "%d", i);
    fvec grad;
    loss = compute_gradient(grad, vecs, labels);
    if (i % 10 == 0) fprintf(stderr, "Avg loss: %f\n", loss);
    
    for (j=0; j <= fdim; j++) {
      alpha[j] += rate * grad[j];
    }
  }
  fprintf(stderr, " Done.\n");
	 
}

double LogReg::eval(std::vector<fvec> &vecs, std::vector<int> &labels) {
  double v; 
  int corr = 0;
  int i;
  for (i=0; i < labels.size(); i++) {
    v = predict(vecs[i]);
    if (v > 0.5 && labels[i]== 1)
      corr++;
    else if (v < 0.5 && labels[i]==0)
      corr++;

  }
  
  return (double) corr / labels.size();
}


int read_mnist(char * lfile, char *dfile, vector<int> &labels, vector<fvec> &fvecs) {

  ifstream fin;
  fin.open(lfile, ios::binary | ios::in);
  int32_t x;
  char c;
  unsigned char val;
  int i,j;
  fin.read((char*) &x, sizeof(int32_t));
  fin.read((char*) &x, sizeof(int32_t));
  x = __builtin_bswap32 (x);

  ifstream din;
  int32_t n, rows, cols;
  din.open(dfile, ios::binary | ios::in);
  din.read( (char*) &n, sizeof(int32_t));
  din.read( (char*) &n, sizeof(int32_t));
  n = __builtin_bswap32 (n);

  din.read( (char*) &rows, sizeof(int32_t));
  rows = __builtin_bswap32 (rows);

  din.read( (char*) &cols, sizeof(int32_t));
  cols = __builtin_bswap32 (cols);
  

  fprintf(stderr, "Reading %d labels\n", (unsigned int) x);

  fvec v;
  for (i=0; i < x; i++) {
    fin.read((char*) &c, sizeof(int8_t));
    
    //fprintf(stderr, "Found : %d\n", (int) c);
    v[0]=1.0;

    for (j=0; j < rows*cols; j++) {
      din.read((char*) &val, sizeof(int8_t));
      if (val > 0)
	v[j+1]=((unsigned int) val ) / 255.0;
      
      //      fprintf(stderr, "Found : %f\n", v[j]);
    }

    if ((int) c <=1 ) {
      labels.push_back((int) c);
      
      fvecs.push_back(v);

    }

	  
  }
  fin.close();
 
  
  // x << f; // magic number

  return rows*cols;
}



int main(int argc, char** argv) {

  if (argc < 7) {
    fprintf(stderr, "logreg [mnist-train]\n");
    return -1;
  }
    
  char * mnist = argv[1];
  char * mnist2 = argv[2];

  char * tmnist = argv[3];
  char * tmnist2 = argv[4];
      
  int iterations = atoi(argv[5]);
  double epsilon = strtod(argv[6], 0);
  
  vector<int> labels;
  vector<fvec> data;

  vector<int> tlabels;
  vector<fvec> tdata;

  int fdim = read_mnist(mnist, mnist2, labels, data);


  fprintf(stderr, "Read %d training examples\n", labels.size());

  int fdim2 = read_mnist(tmnist, tmnist2, tlabels, tdata);

  fprintf(stderr, "Read %d test examples\n", tlabels.size());
  

  LogReg lr(fdim, epsilon);

  lr.train(iterations, data, labels);

  double acc = lr.eval(tdata, tlabels);
  

  printf("Test accuracy %0.2f\n", acc);

  return 0;
}

