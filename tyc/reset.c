/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <getopt.h>
#include "ty.h"
#include "main.h"

static const char *short_options = MAIN_SHORT_OPTIONS "b";
static const struct option long_options[] = {
    MAIN_LONG_OPTIONS

    {"bootloader", no_argument, NULL, 'b'},
    {0}
};

static bool bootloader = false;

void print_reset_usage(FILE *f)
{
    fprintf(f, "usage: tyc reset\n\n");

    print_main_options(f);
    fprintf(f, "\n");

    fprintf(f, "Reset options:\n"
               "   -b, --bootloader         Switch board to bootloader\n");
}

int reset(int argc, char *argv[])
{
    tyb_board *board;
    int r;

    int c;
    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
        case 'b':
            bootloader = true;
            break;

        default:
            r = parse_main_option(argc, argv, c);
            if (r <= 0)
                return r;
            break;
        }
    }

    if (argc > optind) {
        ty_error(TY_ERROR_PARAM, "No positional argument is allowed");
        goto usage;
    }

    r = get_board(&board);
    if (r < 0)
        return r;

    if ((bootloader || !tyb_board_has_capability(board, TYB_BOARD_CAPABILITY_RESET))
            && tyb_board_has_capability(board, TYB_BOARD_CAPABILITY_REBOOT)) {
        printf("Triggering board reboot\n");
        r = tyb_board_reboot(board);
        if (r < 0)
            goto cleanup;

        r = tyb_board_wait_for(board, TYB_BOARD_CAPABILITY_RESET, -1);
        if (r < 0)
            goto cleanup;
    }

    if (!bootloader) {
        if (!tyb_board_has_capability(board, TYB_BOARD_CAPABILITY_RESET)) {
            r = ty_error(TY_ERROR_MODE, "No way to trigger reset for this board");
            goto cleanup;
        }

        printf("Sending reset command\n");
        r = tyb_board_reset(board);
        if (r < 0)
            goto cleanup;
    }

    r = 0;
cleanup:
    tyb_board_unref(board);
    return r;

usage:
    print_reset_usage(stderr);
    return TY_ERROR_PARAM;
}