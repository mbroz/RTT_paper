//
// Created by user on 26/10/2023.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "biased_gen.h"

void print_array(unsigned char* array, int size){
    int i = 0;
    printf("\n");
    for(i = 0; i < size; i++)
    {
        printf("%02x ", array[i]);
    }
    printf("\n");
    fflush(stdin);
}

static int help(const char *arg0, const char *err) {
    if (err)
        printf("%s\n\n", err);
    printf("Use: %s --file=<output> [--size=<MBytes>] --chi2=<num> --swaps=<num> [--blocksize=<num>]\n", arg0);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    unsigned char *output, *reference;
    int i, num_blocks, block_size, num_swaps, *Oi, num_bins;
    char opt[129], val[129], *file, *end;
    unsigned long long size;
    double chi2;
    FILE *fp;

    // defaults
    file = NULL;
    block_size = 1;
    num_swaps = 0;
    chi2 = 1000.;
    size = 400 * 1024 *1024;

    for (i = 1; i < argc; i++) {
        if (sscanf(argv[i], "--%128[^=]=%128s", opt, val) != 2)
            return help(argv[0], NULL);

        if (!strcmp(opt, "file")) {
            file = strdup(val);
        } else if (!strcmp(opt, "size")) {
            size = strtoull(val, &end, 10);
            if (*end || errno == ERANGE)
                return help(argv[0], "Invalid file size.");
        } else if (!strcmp(opt, "blocksize")) {
            block_size = strtol(val, &end, 10);
            if (*end || errno == ERANGE)
                return help(argv[0], "Invalid block size.");
        } else if (!strcmp(opt, "chi2")) {
            chi2 = strtod(val, &end);
            if (*end || errno == ERANGE)
                return help(argv[0], "Invalid chi2.");
        } else if (!strcmp(opt, "swaps")) {
            num_swaps = strtol(val, &end, 10);
            if (*end || errno == ERANGE)
                return help(argv[0], "Invalid swaps.");
        }
    }

    if (!file)
        return help(argv[0], "File parameter is mandatory.");

    printf("PARAMS: file %s, size %llu, blocksize %i, chi2 %f, swaps %i\n",
           file, size, block_size, chi2, num_swaps);

    num_blocks = size / block_size;
    num_bins = 1 << (block_size * 8);

    reference = malloc(block_size * num_blocks);
    output = malloc(block_size * num_blocks);
    Oi = malloc(num_bins * sizeof(int));

    if (!reference || !output || !Oi) {
        printf("Cannot allocate memory.\n");
        return EXIT_FAILURE;
    }

    gen_freqs(1000, block_size, num_blocks, Oi);

    basic_dist(Oi, block_size, num_blocks, reference);
//    print_array(reference, num_blocks*block_size);

    shuffling(reference, block_size, num_blocks, num_swaps, output);
//    print_array(output, num_blocks*block_size);

//    random_selection(reference, block_size, num_blocks, output);
//    print_array(output, num_blocks*block_size);

    fp = fopen(file, "w");
    if (!fp) {
        printf("File %s cannot be created.\n", file);
        return EXIT_FAILURE;
    }

    if (fwrite(output, size, 1, fp) != 1)
        printf("Error writing file %s.\n", file);

    fclose(fp);

    free(file);
    free(output);
    free(reference);
    free(Oi);

    return EXIT_SUCCESS;
}
