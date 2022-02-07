#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "ari.h"
#include "ppm.h"

#define SUBMIT 1

int main(int argc, char **argv) {
#if !SUBMIT
  compress_ppm("C:\\Users\\1\\Documents\\EVM\\compression_ari_4\\my_tests\\public_tests_programs\\clang-tidy.exe", "2.txt");
  decompress_ppm("2.txt", "3.txt");
  return 0;
#else
  CompressOptions *opts = parse_args(argc, argv);
  if (opts != NULL) {
        if (opts->mode == 'c') {
            if (opts->method == ARI) {
                compress_ari(opts->ifile, opts->ofile);
            }
            else if (opts->method == PPM) {
                compress_ppm(opts->ifile, opts->ofile);
            }
        }
        else if (opts->mode == 'd') {
            if (opts->method == ARI) {
                decompress_ari(opts->ifile, opts->ofile);
            }
            else if (opts->method == PPM) {
                decompress_ppm(opts->ifile, opts->ofile);
            }
        }
    }
    free_compress_opts(opts);
    return 0;
#endif
}
