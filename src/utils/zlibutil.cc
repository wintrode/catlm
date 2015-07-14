#include <zlib.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define CHUNK 0x100
#define OUT_CHUNK CHUNK*100
unsigned char gzip_in[CHUNK];
unsigned char gzip_out[OUT_CHUNK+1];
int is_open=0;

char* first_line=(char*)&gzip_out[0];
char* current_line=first_line;
char* next_line=first_line;
char hangover[1000];


bool readLine(FILE* infile,gzFile infd, char* line, int bsize){
  char *tmp; int len;
  
  if (infile) {
    tmp = fgets(line, bsize, infile);
    // replace newline w/ \0
    if (tmp == NULL)
      return false;
    
  }
  else {
    tmp = gzgets(infd,line, bsize); 
    if (tmp == NULL) {
      is_open=0;
      return false;
    }
  }
  len = strlen(line);
  if (len > 0 && line[len-1] == '\n') 
    line[len-1]=0;
    
  return tmp != NULL;
}
