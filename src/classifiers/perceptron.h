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

#include <math.h>

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

    bool fixedA0;
    double epsilon; // learning rate

  public:
    int ccorrect;
    int tclass;

    Perceptron(int dim, double epsilon=0.1);
    ~Perceptron();

    double get_alpha(int i) {
      if (i >= 0 && i <= fmax)
        return alpha[i];
      else
        return 0.0;
    }

    int get_fmax() { return fmax;}

    double score_example(fvec &hyp, fvec &dif);

    double score_example(fvec &hyp);
    
    double l1Diff(fvec &hyp, fvec &truth) {
      double score = 0.0;
      fiterator it;
      for (it = hyp.begin(); it != hyp.end(); it++) {
        if (it->first >= 0 && it->first <= fmax) {
          if (truth.count(it->first) > 0)
            score += fabs(it->second - truth[it->first]);
          else 
            score += fabs(it->second);
        }
        
      }
      for (it = truth.begin(); it != truth.end(); it++) {
        if (hyp.count(it->first) == 0) // already got it
          score += fabs(it->second);
      }

      return score;
    }

    

    void reset_status() {
      ccorrect = 0;
      tclass = 0;
    }

    void update_param(fvec &truth, fvec &hyp, int minid=0);

    void update_param(double truth, fvec &hyp, int minid=0);

    void update_param(fvec &delta, int minid=0);

    bool read(FILE *fp);
    void write(FILE *fp);

    void setFixedA0(bool _fixedA0) {
      fixedA0=_fixedA0;
    }

    void setAlpha0(double alpha0) {
      if (fmax >= 0) {
        alpha[0] = alpha0;
        amean[0] = alpha0;
      }
    }

  };

}



#endif
