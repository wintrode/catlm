bin_PROGRAMS = catlm_train catlm_infer triglm_train

AM_CXXFLAGS = $(CATLM_CFLAGS)

catlm_train_SOURCES = train_main.cc corpus/corpus.cc utils/utils.cc
catlm_infer_SOURCES = infer_main.cc corpus/corpus.cc utils/utils.cc
catlm_train_LDADD = $(CATLM_LIBS)
catlm_infer_LDADD = $(CATLM_LIBS)

triglm_train_SOURCES = triglm.cc utils/vocab_trie.cc
