#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define min(a,b) ((a)<(b)?(a):(b))
#define BUFSIZE       8192

/* pipe to a tcp connection to jaypack-server */

typedef long long llong;
typedef unsigned char byte;

byte buf[BUFSIZE];

int main(int argc, char** argv)
{
  char ms[256];
  if (argc < 4) {
    fprintf(stderr, "Usage: %s blockdevice offset extrasize\n", argv[0]);
    return(0);

  }

  char* blockdevice = argv[1];
  llong offset = atoll(argv[2]),
        plusize = atoll(argv[3]);

  FILE * f = fopen(blockdevice, "rb");
  if (!f) {
    fprintf(stderr, " could not open %s\n", blockdevice);
    return(-1);
  }

  while (!feof(stdin)) {
    llong off, count;
    fscanf(stdin, "%255s %lld %lld\n", ms, &off, &count);
    off += offset;
    count += plusize;

    fseek(f, off, SEEK_SET);
    
    size_t rd = 0, srd;

    uint64_t netdata[2] = { off, count };
    fwrite(netdata, sizeof(netdata), 1, stdout);
    fflush(stdout);

    while ((rd += (srd = fread(buf, 1, min(BUFSIZE, count - rd), f))) < count) {
      if (srd == 0) {
        fprintf(stderr, "unexpected end of file @ %lld", off + rd);
        return(-2);
      }
      fwrite(buf, 1, srd, stdout);
      fflush(stdout);
    }
    if (srd != 0) {
      fwrite(buf, 1, srd, stdout);
      fflush(stdout);
    }
    

  }
  fclose(f);
  return(0);
}

