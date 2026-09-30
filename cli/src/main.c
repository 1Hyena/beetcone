// SPDX-License-Identifier: MIT
#include "../../beetcone.h"
#include "utils.h"

static int do_help  (const char *);
static int do_base58(const char *);
static int do_pubkey(const char *);
static int do_addr  (const char *);

static void print_usage(FILE *);

static const struct command_type {
    const char *name;
    int (*fun) (const char *);
} command_table[] = {
    { .name = "help",   .fun    = do_help   },
    { .name = "base58", .fun    = do_base58 },
    { .name = "pubkey", .fun    = do_pubkey },
    { .name = "addr",   .fun    = do_addr   }
};

static constexpr size_t COMMAND_COUNT = (
    sizeof(command_table)/sizeof(command_table[0])
);

struct {
    char **argv;
    int argc;
} global;

int main(int argc, char *argv[]) {
    global.argv = argv;
    global.argc = argc;

    int opt;

    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h': {
                print_usage(stdout);
                return EXIT_SUCCESS;
            }
            default: {
                print_usage(stderr);
                return EXIT_FAILURE;
            }
        }
    }

    const char *command = optind < argc ? argv[optind++] : nullptr;
    const char *argument = optind < argc ? argv[optind] : "-";

    if (!command) {
        fprintf(stderr, "error: %s\n", "expected command name");
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    for (size_t i=0; i<COMMAND_COUNT; ++i) {
        if (strcasecmp(command, command_table[i].name)) {
            continue;
        }

        return command_table[i].fun(argument);
    }

    fprintf(stderr, "unknown command: %s\n", command);

    return EXIT_FAILURE;
}

static int do_help(const char *argument) {
    print_usage(stdout);

    return EXIT_SUCCESS;
}


static int do_base58(const char *argument) {
    size_t bufsz = beetcone_base58_estimate_strlen(
        argument, strlen(argument)
    ) + 1;

    char buf[bufsz];

    beetcone_base58_encode(argument, strlen(argument), buf, bufsz);

    printf("%s\n", buf);

    return EXIT_SUCCESS;
}

static int do_pubkey(const char *argument) {
    auto key = beetcone_make_key(argument, strlen(argument));

    if (!key.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    auto pubkey = beetcone_key2pubkey(key);

    if (!pubkey.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < sizeof(pubkey.data); ++i) {
        printf("%02x", pubkey.data[i]);
    }

    printf("%s", "\n");

    return EXIT_SUCCESS;
}

static int do_addr(const char *argument) {
    auto key = beetcone_make_key(argument, strlen(argument));

    if (!key.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    auto addr = beetcone_key2addr(key);

    if (!addr.bitset.valid) {
        fprintf(stderr, "invalid argument: %s\n", argument);
        return EXIT_FAILURE;
    }

    printf("%s\n", addr.base58);

    return EXIT_SUCCESS;
}

static void print_usage(FILE *fp) {
    fprintf(
        fp,
        "usage:  %s %s %s %s\n"
        "\n"
        "With no ARGUMENT, or when ARGUMENT is @-, read standard input.\n"
        "If ARGUMENT begins with @, then treat it as a file path and read it.\n"
        "\n"
        "Available options:\n"
        "  -h                    display this help and exit\n",
        global.argv[0], "[OPTION]...", "<COMMAND>", "[ARGUMENT]"
    );

    fprintf(fp, "%s", "\nAvailable commands:\n");

    for (size_t i=0; i<COMMAND_COUNT; ++i) {
        fprintf(fp, "    %s\n", command_table[i].name);
    }
}
