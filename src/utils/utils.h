// utils/utils.h

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

#ifndef CATLM_UTILS_UTILS_H
#define CATLM_UTILS_UTILS_H

#include <string>
#include <map>
#include <vector>

#include <iostream>
#include <fstream>

#include <set>
#include <map>
#include <vector>
#include <string>

#include <stdio.h>

int write_map(std::string &filename, std::map<int, double>& strmap,
	      std::vector<std::string>& vocab);

int read_counts(std::string &filename, std::map<int, double>& strmap,
		int(*idlookup)(std::string&));

int read_map(std::string &filename, std::map<int, double>& strmap, 
	     int(*idlookup)(std::string&));

int read_map(std::string &filename, std::map<std::string, int>& strmap, 
	     std::vector<std::string> *idlist);

int read_set(std::string &filename, std::set<int>& strset, 
	     int(*idlookup)(std::string&));

#endif
