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



#ifndef __FSP_HEADER_H__
#define __FSP_HEADER_H__


#define CHUNK_SIZE 1024


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>


extern const char *__progname;

struct Split_Options
{
    char        *filename;
    char        *output;
    bool        compress;
    size_t      block_size;
};


struct Join_Options
{
    char        *dirname;
    char        *output;
    bool        compress;
};


struct Options {
    struct Split_Options    *split_options;
    struct Join_Options     *join_options;
};


struct Block
{
    size_t      block_size;
    size_t      original_size;
    bool        compression;
    void        *data;
};


void help_menu();
size_t parse_block_size(char *size, size_t len);
struct Options *parse_arguments(int argc, char **argv);
void split_file(struct Split_Options *options);
void join_files(struct Join_Options *options);
void check_directory(char *dirname);
struct Block *compress_data(char *data, size_t len);
struct Block *decompress_data(struct Block *data);
void write_block(struct Block *block, int fd);
struct Block *read_block(int fd);
bool endswith(char *str, char *key);

#endif