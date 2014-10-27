#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 10LL * 1024LL * 1024LL
#define BUFSIZE 8192
#define SKIPBYTES 0
#define RSSIZE  16

typedef unsigned char byte;
typedef long long llong;
typedef unsigned long long ullong;

byte buf[BUFSIZE], rotostring[RSSIZE], *bufptr;
int maxbuf, rsi;

int read_buf()
{
  size_t out = fread(buf, 1, BUFSIZE, stdin);
  bufptr = buf;
  maxbuf = out;
  if (out == 0)
    return(-1);
  else return(out);
}

int next_byte()
{
  if (bufptr > buf + maxbuf - 1) {
    if (read_buf() < 0)
      return(-1);
  }
  return *bufptr++;
}

void push_rotobyte(byte b)
{
  int nrsi = (rsi + 1) % RSSIZE;
  rotostring[nrsi] = b;
  rsi = nrsi;
}

int test_rotostring(const byte s[], const byte mask[], size_t sz)
{
  /* assert sz <= RSSIZE */
  int i;
  for (i = 0; i < sz; ++i) {
    if (mask[sz - 1 - i] && s[sz - 1 - i] != rotostring[(rsi - i + RSSIZE) % RSSIZE])
      return(0);
  }
  return(1);
}

byte start_str [] = { 0xFF, 0xD8, 0xFF },
     end_str   [] = { 0xFF, 0xD9 },
     end_mask  [] = { 0x1 , 0x1  },
     /* 0xEE are unimportant bytes */
     jfif_str  [] = { 0xFF, 0xD8, 0xFF, 0xE0, 0xEE, 0xEE, 'J', 'F', 'I', 'F', 0x00 },
     jfif_mask [] = { 0x1 , 0x1,  0x1,  0x1,  0x0,  0x0,  0x1, 0x1, 0x1, 0x1, 0x1  },
     spiff_str [] = { 0xFF, 0xD8, 0xFF, 0xE8, 0xEE, 0xEE, 'S', 'P', 'I', 'F', 'F', 0x00 },
     spiff_mask[] = { 0x1 , 0x1,  0x1,  0x1,  0x0,  0x0,  0x1, 0x1, 0x1, 0x1, 0x1, 0x1  },
     exif_str  [] = { 0xFF, 0xD8, 0xFF, 0xE1, 0xEE, 0xEE, 'E', 'x', 'i', 'f', 0x00 },
     exif_mask [] = { 0x1 , 0x1,  0x1,  0x1,  0x0,  0x0,  0x1, 0x1, 0x1, 0x1, 0x1  };

typedef struct header_str {
  byte * str,
       * mask;
  char * fmtid;
  int    len;
} header_str;

header_str headerstrings[] = {
  { jfif_str, jfif_mask, "jfif", sizeof(jfif_str) },
  { spiff_str, spiff_mask, "spiff", sizeof(spiff_str) },
  { exif_str, exif_mask, "exif", sizeof(exif_str) }
};

#define HEADER_STRING_COUNT       (sizeof(headerstrings) / sizeof(header_str))

int opt_maxsz,
    opt_skipbytes;

void read_through()
{
  int c;
  llong l2skip = 0, soi = -1, cursz = 0, off = 0;
  const header_str *hdrstr = NULL;

  void output_img() 
  {
    fprintf(stdout, "%s %lld %lld\n", hdrstr->fmtid, soi + 1 - hdrstr->len, cursz);
    fflush(stdout);
  }

  while ((c = next_byte()) != -1) {
    push_rotobyte(c);
    if (soi != -1LL) {
      /* too big file! */
      if (cursz >= opt_maxsz) {
        soi = -1LL;
        cursz = 0;
        l2skip = 0;
        hdrstr = NULL; 

        ++off;
        continue;
      }

      if (l2skip <= 0LL) {
        if (test_rotostring(end_str, end_mask, sizeof(end_str))) {
          /* eoi found! check size... */
          if (cursz <= opt_maxsz) {
            output_img();

            soi = -1LL;
            cursz = 0;
            l2skip = 0;
            hdrstr = NULL;

            ++off;
            continue;
          }
        }
      } else {
        --l2skip;
      }

      ++cursz;
    }

    int i;
    for (i = 0; i < HEADER_STRING_COUNT; ++i) {
      const header_str* hs = headerstrings + i;
      if (test_rotostring(hs->str, hs->mask, hs->len)) { 
        if (soi != -1LL) { /* we were inside an image before, dump it */
          output_img();
        }

        /* SOI found! */
        soi = off;
        cursz = 0LL;
        l2skip = opt_skipbytes;
        hdrstr = hs;
      }
    }
    ++off;
  }
}

int main(int argc, char** argv)
{
  if (argc < 3) {
    fprintf(stderr,
        "Usage: \n"
        "\n"
        "    %s maxsize skipbytes\n"
        "\n"
        "  maxsize   - The maximum size of the file to consider, or a single minus\n"
        "              sign. If the latter, 10MiB is assumed.\n"
        "  skipbytes - The number of bytes to skip and not check for EOI after SOI is\n"
        "              encountered. Since we're still not smart enough to actually\n"
        "              skip frames and check for EOI only in the image data (which is\n"
        "              guaranteed not to contain a spurious EOI mark in compressed data\n"
        "              and can safely be searched), we need to skip some amount of bytes\n"
        "              in order to prevent something that looks like an EOI from triggering\n"
        "              us. Note, this is not likely to happen, just possible. A minus sign\n"
        "              instead of a value will default to 0.\n"
        "\n",
        argv[0]
    );
    return(0);
  }

  opt_maxsz = (strcmp(argv[1], "-") == 0) ? MAXSIZE : atoi(argv[1]);
  opt_skipbytes = (strcmp(argv[2], "-") == 0) ? SKIPBYTES : atoi(argv[2]);
  read_buf();
  read_through();

  return(0);
}

