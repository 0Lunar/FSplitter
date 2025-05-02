#include "fsp.h"

int main(int argc, char **argv)
{
    struct Split_Options *options;

    options = parse_arguments(argc, argv);

    if (options->filename == NULL) {
        help_menu();
        exit(EXIT_FAILURE);
    }

    split_file(options);
}