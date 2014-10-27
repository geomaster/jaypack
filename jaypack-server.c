#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define min(a,b) ((a)<(b)?(a):(b))

#define BUFSIZE       8192
#define FNSIZE        2048

/* pipe to a tcp connection from jaypack-client */

typedef long long llong;
typedef unsigned char byte;

byte buf[BUFSIZE];

int main(int argc, char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s prefix\n", argv[0]);
    return(0);
  }

  char *prefix = argv[1];
  llong totalsaved = 0;

  while (!feof(stdin)) {
    uint64_t netdata[2] = { 0, 0 };
    fread(netdata, sizeof(netdata), 1, stdin);

    llong off = netdata[0], count = netdata[1];

    size_t rd = 0, srd;
    char fname[FNSIZE + 1];
    snprintf(fname, FNSIZE, "%s%lld.jpg", prefix, off);

    FILE * f = fopen(fname, "wb");
    while ((rd += (srd = fread(buf, 1, min(BUFSIZE, count - rd), stdin))) < count) {
      if (srd == 0) {
        fprintf(stderr, "unexpected end of stream\n");
        return(-2);
      }
      fwrite(buf, 1, srd, f);
      fflush(f);
    }
    if (srd != 0) {
      fwrite(buf, 1, srd, f);
      fflush(f);
    }

    totalsaved += count;
    fprintf(stderr, "%-40s %9.2fKB (currently @ %5.2fG, total saved %7.2fM)\n", 
        fname, 
        (double)count / 1024.0,
        (double)off / (1024.0 * 1024.0 * 1024.0),
        (double)totalsaved / (1024.0 * 1024.0)
      );
    fflush(stdout);

    fclose(f);
  }

  return(0);
}
