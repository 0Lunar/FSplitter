#include "fsp.h"

int main(int argc, char **argv)
{
    struct Options *options;

    options = parse_arguments(argc, argv);

    if (options->split_options) {

        if (options->split_options->filename == NULL) {
            help_menu();
            puts("\n\nMissing file option (-f)");
            exit(EXIT_FAILURE);
        }

        split_file(options->split_options);
    }
    else {
        if (options->join_options->output == NULL) {
            help_menu();
            puts("\n\nMissing directory option (-o)");
            exit(EXIT_FAILURE);
        }

        if (options->join_options->dirname == NULL) {
            help_menu();
            puts("\n\nMissing file option (-f)");
            exit(EXIT_FAILURE);
        }

        join_files(options->join_options);
    }
}