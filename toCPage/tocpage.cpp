/************************************************************
 *
 * Copyright (c) 2022 Lian Ferrand
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of I Wanna Slay the Dragon of Bangan
 *
 ************************************************************/

#include <cstdio>
#include <cstdlib>

#define ERR(condition, msg) if (condition) {puts(msg); exit(1);}

static unsigned s_imageRow[2048];

static const char *GetFilename(int argc, char **argv) {
  if (argc <= 1) return NULL;
  return argv[1];
}

union block_t {
  unsigned pixel;

  unsigned markerLength;

  struct {
    unsigned char padding[3];
    unsigned char magic; // == 0x80

    unsigned pixel;
  } marker;
};

static void WriteBlock(FILE *out, block_t *b) {
  if (b->marker.magic == 0x80)
    fwrite(b, 1, 8, out);
  else
    fwrite(b, 1, 4, out);
}

#define FlushLastBlock(out) WritePixel(out, 0, true)
static void WritePixel(FILE *out, unsigned pixel, bool flushBlock = false) {
  static block_t curBlock;
  static bool compressing = false;

  if (flushBlock) {
    WriteBlock(out, &curBlock);
    return;
  }

  if (!compressing) {
    curBlock.pixel = pixel;
    ERR(curBlock.marker.magic == 0x80, "A pixel in this page has the same alpha as the RLE marker magic!");

    compressing = true;
    return;
  }

  if (curBlock.marker.magic != 0x80) {
    if (pixel == curBlock.pixel) {
      curBlock.markerLength = 1;
      curBlock.marker.magic = 0x80;
      curBlock.marker.pixel = pixel;
      return;
    }

    WriteBlock(out, &curBlock);
    curBlock.pixel = pixel;
    ERR(curBlock.marker.magic == 0x80, "A pixel in this page has the same alpha as the RLE marker magic!");

    return;
  }

  if (pixel == curBlock.marker.pixel) {
    ERR((curBlock.markerLength&0xffffff) == 0xffffff, "Too much compression!... ?");
    ++curBlock.markerLength; // Little endian dependent, don't care
    return;
  }

  WriteBlock(out, &curBlock);
  curBlock.pixel = pixel;
  ERR(curBlock.marker.magic == 0x80, "A pixel in this page has the same alpha as the RLE marker magic!");
}

int main(int argc, char **argv) {
  if (!GetFilename(argc, argv)) {
    printf("Usage: %s <page number>\n"
           "\n"
           "Converts <page>.bmp to ../<page>\n", argv[0]);
    return 0;
  }

  char name[6] = " .bmp";
  char outName[6] = "../ c";
  name[0] = outName[3] = argv[1][0];

  FILE *f = fopen(name, "rb");
  if (!f) {
    printf("Cannot open \"%s\"!\n", name);
    return 1;
  }

  FILE *out = fopen(outName, "wb");
  if (!out) {
    printf("Cannot open \"%s\"!\n", outName);
    fclose(f);
    return 1;
  }

  // Seek to last line
  fseek(f, 2048*2047*4 + 0x46, SEEK_SET);

  // Start reading 2048 lines from f to out
  for (size_t i = 0; i < 2048; ++i) {
    if (fread(s_imageRow, 4, 2048, f) < 2048) {
      puts("Couldn't read image row!");
      fclose(f);
      fclose(out);
      return 1;
    }

    for (size_t j = 0; j < 2048; ++j)
      WritePixel(out, s_imageRow[j]);

    fseek(f, -4096*4, SEEK_CUR);
  }

  FlushLastBlock(out);
  fclose(f);
  fclose(out);

  return 0;
}
