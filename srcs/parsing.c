#include "./../ft_ping.h"

static void settarget(t_data *data, uint8_t *target) {
    data->target = (uint8_t*)strdup((char*)target);
}

static void setIntegerOption(t_data *data, uint8_t opt, uint8_t *arg) {
    if (opt == 'g') {
        data->opts.g = (uint32_t)atoi((char*)arg);
    } else if (opt == 'h') {
        data->opts.h = (uint32_t)atoi((char*)arg);
    } else if (opt == 'G') {
        data->opts.G = (uint32_t)atoi((char*)arg);
    } else if (opt == 's') {
        data->opts.s = (uint32_t)atoi((char*)arg);
    } else if (opt == 'c') {
        data->opts.c = (uint32_t)atoi((char*)arg);
    }
}

static void addOption(t_data *data, uint8_t *opt) {
    uint32_t i = 1;

    while(opt[i]) {
        if (opt[i] == 'h') {
            data->opts.h = 1;
        } else if (opt[i] == 'g') {
            data->opts.g = 1;
        } else if (opt[i] == 'G'){
            data->opts.G = 1;
            data->sweep = true;
        } else if (opt[i] == 'v'){
            data->opts.v = 1;
        } else if (opt[i] == 'a'){
            data->opts.a = true;
        } else if (opt[i] == 'q'){
            data->opts.q = true;
        } else if (opt[i] == 's'){
            data->opts.s = 1;
        } else if (opt[i] == 'c'){
            data->opts.c = 1;
        }
        ++i;
    }
}

static bool isIntegerOption(uint8_t *arg) {
    return (strchr("ghGsc", arg[1]) != NULL);
}

static bool isOption(uint8_t *arg) {
    return (arg[0] == '-');
}

void parsing(t_data *data, uint8_t **argv){
    uint32_t i = 1;

    while(argv[i]) {
        if (isOption(argv[i])) {
            addOption(data, argv[i]);
            if (isIntegerOption(argv[i])) {
                setIntegerOption(data, argv[i][1], argv[i + 1]);
                ++i;
            }
        } else {
            settarget(data, argv[i]);
        }
        ++i;
    }
}