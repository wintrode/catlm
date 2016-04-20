bin_PROGRAMS = catlm_train catlm_infer triglm_train triglm_rescore readlm triglm_train_lattice triglm_rescore_lattice lattice_test
#triglm_lrescore

#AM_CXXFLAGS = $(CATLM_CFLAGS)

#AM_CPPFLAGS=-I${KALDI_ROOT}/src

OPENFSTLIBS = -L${KALDI_ROOT}/tools/openfst/lib -lfst
OPENFSTLDFLAGS = -Wl,-rpath=${KALDI_ROOT}/tools/openfst/lib

FSTROOT = ${KALDI_ROOT}/tools/openfst
ATLASINC = ${KALDI_ROOT}/tools/ATLAS/include
ATLASLIBS = /usr/lib/atlas-base/libatlas.so.3.0 /usr/lib/atlas-base/libf77blas.so.3.0 /usr/lib/atlas-base/libcblas.so.3 /usr/lib/atlas-base/liblapack_atlas.so.3 -Wl,-rpath=/usr/lib/atlas-base

AM_LDFLAGS=$(OPENFSTFLAGS)

AM_CXXFLAGS = -msse -msse2 -Wall -I.. \
	  -fPIC \
      -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
      -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self \
      -DHAVE_EXECINFO_H=1 -rdynamic -DHAVE_CXXABI_H \
      -DHAVE_ATLAS -I$(ATLASINC) \
      -I$(FSTROOT)/include -I$(KALDI_ROOT)/src \
      $(EXTRA_CXXFLAGS) \
      -g # -O0 -DKALDI_PARANOID 



catlm_train_SOURCES = train_main.cc corpus/corpus.cc utils/utils.cc topics/topicmodel.cc
catlm_infer_SOURCES = infer_main.cc corpus/corpus.cc utils/utils.cc topics/topicmodel.cc
catlm_train_LDADD = $(CATLM_LIBS)
catlm_infer_LDADD = $(CATLM_LIBS)

triglm_train_SOURCES = triglm_train.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
triglm_train_LDFLAGS = -lz

triglm_train_lattice_SOURCES = triglm_train_lattice.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
triglm_train_lattice_LDFLAGS = -lz

triglm_rescore_SOURCES = triglm_rescore.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
triglm_rescore_LDFLAGS = -lz

triglm_rescore_lattice_SOURCES = triglm_rescore_lattice.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
triglm_rescore_lattice_LDFLAGS = -lz

readlm_SOURCES = readlm.cc utils/vocab_trie.cc utils/zlibutil.cc
readlm_LDFLAGS = -lz

ADDLIBS = ${KALDI_ROOT}/src/lat/kaldi-lat.a \
		${KALDI_ROOT}/src/hmm/kaldi-hmm.a \
		${KALDI_ROOT}/src/tree/kaldi-tree.a \
	        ${KALDI_ROOT}/src/util/kaldi-util.a \
		${KALDI_ROOT}/src/matrix/kaldi-matrix.a \
		${KALDI_ROOT}/src/thread/kaldi-thread.a \
		${KALDI_ROOT}/src/base/kaldi-base.a

#lattice_test_LINK=$(ADDLIBS)
lattice_test_LDADD=$(ADDLIBS)
lattice_test_SOURCES = lattice_test.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
lattice_test_LDFLAGS = $(OPENFSTLIBS) $(ATLASLIBS) -lm -lpthread -ldl -lz

#triglm_lrescore_LDADD=$(ADDLIBS)
#triglm_lrescore_SOURCES = triglm_lrescore.cc utils/vocab_trie.cc classifiers/perceptron.cc utils/zlibutil.cc utils/utils.cc asr/triglm.cc 
#triglm_lrescore_LDFLAGS = $(OPENFSTLIBS) $(ATLASLIBS) -lm -lpthread -ldl -lz
