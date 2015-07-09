// utils/zlibutil.h

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

#ifndef CATLM_UTILS_ZLIBUTIL_H
#define CATLM_UTILS_ZLIBUTIL_H

#include <zlib.h>

z_stream init_gzip_stream(FILE* file,char* out);
bool inflate_gzip(FILE* file, z_stream strm,size_t bytes_read);
int readLine(FILE* infile,char* line,bool gzipped,int bsize);


#endif
