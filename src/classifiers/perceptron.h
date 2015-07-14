// classifiers/perceptron.h

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

#ifndef CATLM_CLASSIFIERS_PERCEPTRON_H
#define CATLM_CLASSIFIERS_PERCEPTRON_H


#include <string>
#include <map>
#include <vector>

typedef std::map<int, double> fvec;
typedef std::map<int, double>::iterator fiterator;

namespace catlm {

  using std::string;


  class Perceptron {

  private:
    double *alpha;
    double *amean;
    int N;
    int fmax;


  public:
    Perceptron(int dim);
    ~Perceptron();

    double get_alpha(int i) {
      if (i >= 0 && i <= fmax)
        return alpha[i];
      else
        return 0.0;
    }

    double score_example(fvec &hyp) {
      double score = 0.0;
      fiterator it;
      for (it = hyp.begin(); it != hyp.end(); it++) {
        if (it->first >= 0 && it->first <= fmax)
          score += it->second * alpha[it->first];
      }
      return score;
    }

    void update_param(fvec &truth, fvec &hyp);

    bool read(FILE *fp);
    void write(FILE *fp);

  };

}



#endif
