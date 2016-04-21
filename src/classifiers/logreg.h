// classifiers/logreg.h

// Copyright 2016 Jonathan Wintrode


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

#ifndef CATLM_CLASSIFIERS_LOGREG_H
#define CATLM_CLASSIFIERS_LOGREG_H


#include <string>
#include <map>
#include <vector>

#include <math.h>

typedef std::map<int, double> fvec;
typedef std::map<int, double>::iterator fiterator;


namespace catlm {

  using std::string;


  class LogReg {

  private:
    double *alpha;
    int fdim;

    double rate;
    
  public:
    double predict(fvec &input);
    LogReg(int fdim, double rate=0.1);
    LogReg() { };
    ~LogReg();

    double compute_gradient(fvec &grad, std::vector<fvec> &vecs, std::vector<int> &labels);
    void train(int iterations, std::vector<fvec> &vecs, std::vector<int> &labels);

    double eval(std::vector<fvec> &vecs, std::vector<int> &labels);
  };

}

#endif
