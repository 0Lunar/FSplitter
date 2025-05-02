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


void split_file(struct Split_Options *options) {
    int fd_src, fd_dst;
    size_t rd, rd_tot;
    int cnt;
    char buff[CHUNK_SIZE];
    char new_file[CHUNK_SIZE];
    char file_count[16];
    struct Block *block;


    if (access(options->filename, R_OK) != 0) {
        fprintf(stderr, "Permissions error on the file: %s", options->filename);
        exit(EXIT_FAILURE);
    }

    check_directory(options->output);

    fd_src = open(options->filename, O_RDONLY);
    cnt = 0;
    rd = 0;
    rd_tot = 0xffffffffffffffff;

    do {
        memset(buff, 0, CHUNK_SIZE);

        rd = read(fd_src, buff, CHUNK_SIZE);

        if (rd_tot >= options->block_size || rd_tot == 0xffffffffffffffff) {
            sprintf(file_count, "%d", cnt);
            strcpy(new_file, options->output);
            strcat(new_file, "/block_");
            strcat(new_file, file_count);
            strcat(new_file, ".fsp");

            if (rd_tot > 0) {
                close(fd_dst);
            }

            fd_dst = open(new_file, O_CREAT | O_WRONLY, 0644);

            if (fd_dst < 0) {
                fprintf(stderr, "Error opening: %s", new_file);
                exit(EXIT_FAILURE);
            }

            rd_tot = 0;
            cnt++;
        }
        

        if (options->compress) {
            block = compress_data(buff, rd);
            rd_tot += block->block_size;
        }
        else {
            block = malloc(sizeof(struct Block));
            block->block_size = rd;
            block->compression = false;
            block->data = buff;
            rd_tot += rd;
        }

        write_block(block, fd_dst);
    } while ((rd == CHUNK_SIZE));
}


void write_block(struct Block *block, int fd) {
    write(fd, &block->block_size, sizeof(size_t));
    write(fd, &block->compression, sizeof(bool));
    write(fd, block->data, block->block_size);
}