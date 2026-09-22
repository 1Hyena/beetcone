// SPDX-License-Identifier: MIT
#include "../../beetcone.h"
#include "utils.h"

static int do_help  (const char *);
static int do_base58(const char *);
static int do_pubkey(const char *);
static int do_addr  (const char *);

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage:  %s %s %s\n", argv[0], "command", "[args]");
        do_help("");

        return EXIT_FAILURE;
    }

    const char *command = argv[1];
    const char *argument = argc == 2 ? "" : argv[2];

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
    printf("%s", "commands:\n");

    for (size_t i=0; i<COMMAND_COUNT; ++i) {
        printf("    %s\n", command_table[i].name);
    }

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
