#include <zlib.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#define CHUNK 0x100
#define OUT_CHUNK CHUNK*100
unsigned char gzip_in[CHUNK];
unsigned char gzip_out[OUT_CHUNK];
///* These are parameters to inflateInit2. See http://zlib.net/manual.html for the exact meanings. */
#define windowBits 15
#define ENABLE_ZLIB_GZIP 32
z_stream strm = {0};
z_stream init_gzip_stream(FILE* file,char* out){// unsigned     
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.next_in = gzip_in;
  strm.avail_in = 0;
  strm.next_out = gzip_out;
  inflateInit2 (& strm, windowBits | ENABLE_ZLIB_GZIP);
  return strm;
}

bool inflate_gzip(FILE* file, z_stream strm,size_t bytes_read){
  strm.avail_in = (int)bytes_read;
  do {
    strm.avail_out = OUT_CHUNK;
    inflate (& strm, Z_NO_FLUSH);
    //              printf ("%s",gzip_out);
  }while (strm.avail_out == 0);
  if (feof (file)) {
    inflateEnd (& strm);
    return false;
  }
  return true;// all OK
}


char* first_line=(char*)&gzip_out[0];
char* current_line=first_line;
char* next_line=first_line;
char hangover[1000];

int readLine(FILE* infile,char* line,bool gzipped, int bsize){
  char *tmp; int len;
  if(!gzipped) {
    tmp = fgets(line, bsize, infile);
    // replace newline w/ \0
    len = strlen(line);
    if (len > 0 && line[len-1] == '\n') 
      line[len-1]=0;

    return tmp != NULL;
  }
  else {
    int ok=1; //true;
    current_line=next_line;
    if(!current_line || strlen(current_line)==0 || next_line-current_line>OUT_CHUNK){
      current_line=first_line;
      size_t bytes_read = fread (gzip_in, sizeof (char), CHUNK, infile);
      if (!inflate_gzip(infile,strm,bytes_read))
	ok = 0;
      strcpy(line,hangover);
    }
    if(ok){
      next_line=strstr(current_line,"\n");
      if(next_line){
	next_line[0]=0;
	next_line++;
	strcpy(line+strlen(hangover),current_line);
	hangover[0]=0;
      }else{
	strcpy(hangover,current_line);
	line[0]=0;// skip that one!!
	//fprintf(stdout, "blank\n");
	ok = 2;
      }
    }
    return ok;
  }
}
