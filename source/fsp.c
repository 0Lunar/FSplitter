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


    if (access(options->filename, F_OK) != 0) {
        fprintf(stderr, "Missing file: %s", options->filename);
        exit(EXIT_FAILURE);
    }

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
                block = malloc(sizeof(struct Block));
                block->block_size = 0;
                block->original_size = 0;
                block->compression = false;
                block->data = NULL;

                write_block(block, fd_dst);

                free(block);

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
            block->original_size = rd;
            block->compression = false;
            block->data = buff;
            rd_tot += rd;
        }

        write_block(block, fd_dst);
        free(block);
    } while ((rd == CHUNK_SIZE));
}


void join_files(struct Join_Options *options) {
    struct Block *block;
    int fd_dst, fd_src;
    int f_cnt;
    char filename[CHUNK_SIZE];
    char cnt_to_str[16];

    if (access(options->dirname, R_OK) != 0) {
        fprintf(stderr, "Permissions error on the directory: %s", options->dirname);
        exit(EXIT_FAILURE);
    }

    fd_dst = open(options->output, O_CREAT | O_WRONLY, 0666);

    if (fd_dst < 0) {
        fprintf(stderr, "Error opening: %s", options->output);
        exit(EXIT_FAILURE);
    }

    f_cnt = 0;

    while (true) {
        strcpy(filename, options->dirname);
        strcat(filename, "/");
        strcat(filename, "block_");
        sprintf(cnt_to_str, "%d", f_cnt);
        strcat(filename, cnt_to_str);
        strcat(filename, ".fsp");

        if (access(filename, F_OK) != 0) {
            break;
        }

        if (access(filename, R_OK) != 0) {
            fprintf(stderr, "Error opening: %s", options->dirname);
            exit(EXIT_FAILURE);
        }

        fd_src = open(filename, O_RDONLY);

        if (fd_src < 0) {
            fprintf(stderr, "Error opening: %s", options->dirname);
            exit(EXIT_FAILURE);
        }

        while (true) {
            block = read_block(fd_src);

            if (block->block_size == 0) {
                break;
            }

            if (block->compression) {
                if ((block = decompress_data(block)) == NULL) {
                    fprintf(stderr, "Error decompressing %s", filename);
                    exit(EXIT_FAILURE);
                }
            }

            write(fd_dst, block->data, block->block_size);

            if (block->original_size < CHUNK_SIZE) {
                free(block);
                break;
            }

            free(block);
        }
        close(fd_src);

        f_cnt++;
    };
}


void write_block(struct Block *block, int fd) {
    write(fd, &block->block_size, sizeof(size_t));
    write(fd, &block->original_size, sizeof(size_t));
    write(fd, &block->compression, sizeof(bool));

    if (block->data != NULL && block->block_size != 0) {
        write(fd, block->data, block->block_size);
    }
}


struct Block *read_block(int fd) {
    struct Block *block;
    size_t rd;

    block = malloc(sizeof(struct Block));

    if (block == NULL) {
        fputs("Error alocating memory", stderr);
        exit(EXIT_FAILURE);
    }

    read(fd, &block->block_size, sizeof(size_t));
    read(fd, &block->original_size, sizeof(size_t));
    read(fd, &block->compression, sizeof(bool));

    if (block->block_size == 0) {
        block->data = NULL;
        return block;
    }

    block->data = malloc(block->block_size);

    if (block->data == NULL) {
        fputs("Error alocating memory", stderr);
        exit(EXIT_FAILURE);
    }

    rd = read(fd, block->data, block->block_size);

    if (rd != block->block_size) {
        fputs("Error reading file", stderr);
        exit(EXIT_FAILURE);
    }

    return block;
}