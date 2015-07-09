#include "topicmodel.h"

#include <stdio.h>


using namespace catlm;


CacheTopicModel::~CacheTopicModel() {

}

void CacheTopicModel::init() {

}


int CacheTopicModel::initFromCorpus(Corpus *corpus) {

  // need bitsets and topic state for each token

}




int CacheTopicModel::estimate(Corpus *corpus, int niterations) {
  int ndoc;
  int iteration=0;
  
  bool printLL = true;

  int totalTokens = initFromCorpus(corpus);
  
  for (iteration=0; iteration < niterations; iteration++) {
    
    
    for (ndoc = 0; ndoc < corpus->size(); ndoc++) {

      sample(corpus->doc_at(ndoc));

    }
    
    // 
    if (iteration > burnin && opt_interval != 0 &&
	iteration % opt_interval == 0) {
      
      optimize_alpha();  // make parallel?
      optimize_beta();
      optimize_nu();

      /*if (nu0 == 0.0) 
	System.err.println("runnables: " + runnables[0].getKappaObservations()[0]);
      */
      
    }

    if (iteration % 10 == 0) {
      if (printLL) {
	fprintf(stderr, "<%d> LL/token: %f\n" + iteration, modelLogLikelihood() / totalTokens);
      }

      if (printTiming) {
	// time per iteration
	fprintf(stderr, "Runtime: %f\n", (double) runtime/10.0);  
	// time per iteration
	fprintf(stderr, "sampling runtime: %f\n", (double) innerRuntime / 10.0);
      }

    }
    

  }

}


int CacheTopicModel::write(const char *fname) {
  return 0;
}


void CacheTopicModel::sample(Document *doc) {

  // need bitsets and topic state for each token

}


double CacheTopicModel::modelLogLikelihood() {
  return 0;
}

void CacheTopicModel::optimize_alpha() {
  
}
void CacheTopicModel::optimize_beta() {
  
}
void CacheTopicModel::optimize_nu() {
  
}
