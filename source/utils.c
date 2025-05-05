/*
MIT License

Copyright (c) 2025 Nicolò Rinaldi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/



#include "fsp.h"


void help_menu()
{
    printf("Usage: ./%s [-h] [-c] [-b <block_size>] [-j] [-o <dir_name>] [-f: <FILE>]\n", __progname);
    puts("Split a file into multiple files.");
    puts("\nFlags:");
    puts("\t-h\t\tShow the help menu and exit");
    puts("\t-c\t\tCompress the output files (default: false)");
    puts("\t-b\t\tSpecify the block size. (default: 4KB)");
    puts("\t-o\t\tOutput directory where the split files will be saved (default: .)");
    puts("\t-f\t\tInput file to be split (required)");
    puts("\t-j\t\tMerge files instead of splitting them");
    puts("\nBlock Flags:");
    puts("\tB\t\tBytes");
    puts("\tK\t\tKilobytes");
    puts("\tM\t\tMegabytes");
    puts("\tG\t\tGigabytes");
    puts("\tT\t\tTerabytes");
}


size_t parse_block_size(char *size, size_t len) {
    size_t blk_size;

    if ((blk_size = (size_t)strtol(size, NULL, 10)) == 0) {
        return 0;
    }

    switch (size[len-1]) {
        case 'K':
        case 'k':
            blk_size <<=  10;
            break;
        
        case 'M':
        case 'm':
            blk_size <<= 20;
            break;

        case 'G':
        case 'g':
            blk_size <<= 30;
            break;

        case 'T':
        case 't':
            blk_size <<= 40;
            break;

        default:
            break;
    }

    return blk_size;
}


struct Options *parse_arguments(int argc, char **argv) {
    struct Split_Options *split_options;
    struct Join_Options *join_options;
    struct Options *options;
    bool join;
    int opt;

    split_options = malloc(sizeof(struct Split_Options));
    options = malloc(sizeof(struct Options));

    split_options->filename = NULL;
    split_options->output = ".";
    split_options->compress = false;
    split_options->block_size = 4096;

    while ((opt = getopt(argc, argv, "jhcb:o:f:")) != -1) {
        switch (opt) {
            case 'h':
                help_menu();
                free(split_options);
                split_options = NULL;
                exit(EXIT_SUCCESS);
                break;
            
            case 'c':
                split_options->compress = true;
                break;
            
            case 'b':
                split_options->block_size = parse_block_size(optarg, strlen(optarg));
                
                if (split_options->block_size == 0) {
                    fputs("Invalid block size", stderr);
                    exit(EXIT_FAILURE);
                }
                break;
            
            case 'o':
                split_options->output = optarg;
                break;
            
            case 'f':
                split_options->filename = malloc(strlen(optarg));
                strcpy(split_options->filename, optarg);
                break;

            case 'j':
                join = true;
                break;
            
            default:
                break;
        }
    }

    if (join) {
        join_options = malloc(sizeof(struct Join_Options));
        join_options->compress = split_options->compress;
        join_options->dirname = split_options->output;
        join_options->output = split_options->filename;

        free(split_options);

        options->join_options = join_options;
        options->split_options = NULL;
    }
    else {
        options->join_options = NULL;
        options->split_options = split_options;
    }

    return options;
}


void check_directory(char *dirname) {
    DIR *dir;

    dir = opendir(dirname);

    if (dir == NULL) {
        if ((mkdir(dirname, 0777)) != 0) {
            fprintf(stderr, "Error creating the directory: %s", dirname);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if (access(dirname, W_OK) != 0) {
            fprintf(stderr, "Can't write on the directory: %s", dirname);
            exit(EXIT_FAILURE);
        }

        closedir(dir);
    }
}


struct Block *compress_data(char *data, size_t len) {
    if (!data || len == 0) return NULL;

    struct Block *new_block = NULL;
    Bytef *comp_buff = NULL;
    uLong comp_len = compressBound(len);

    comp_buff = malloc(comp_len);
    if (!comp_buff) return NULL;

    if (compress(comp_buff, &comp_len, (const Bytef *)data, (uLong)len) != Z_OK) {
        free(comp_buff);
        return NULL;
    }

    new_block = malloc(sizeof(struct Block));
    if (!new_block) {
        free(comp_buff);
        return NULL;
    }

    new_block->block_size = comp_len;
    new_block->original_size = len;
    new_block->compression = true;
    new_block->data = comp_buff;

    return new_block;
}


struct Block *decompress_data(struct Block *data) {
    if (!data) return NULL;

    if (data->original_size == 0) {
        return data;
    }

    struct Block *new_block = NULL;
    Bytef *decomp_buff = NULL;
    uLong decomp_len = data->original_size;

    decomp_buff = malloc(sizeof(Bytef) * decomp_len);

    if (uncompress(decomp_buff, &decomp_len, data->data, data->block_size) != Z_OK) {
        free(decomp_buff);
        return NULL;
    }

    new_block = malloc(sizeof(struct Block));
    if (!new_block) {
        free(decomp_buff);
        return NULL;
    }

    new_block->block_size = data->original_size;
    new_block->original_size = data->original_size;
    new_block->compression = false;
    new_block->data = decomp_buff;

    return new_block;
}


bool endswith(char *str, char *key) {
    int lkey = strlen(key);
    int lstr = strlen(str);

    if (lkey > lstr)
        return false;

    for (int i = 0; i < lkey; i++) {
        if (str[lstr - lkey + i] != key[i])
            return false;
    }

    return true;
}