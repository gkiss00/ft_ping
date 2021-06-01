#include "./../ft_ping.h"

static bool isPositiveIntegerOption(char *arg) {
    return (strchr("c", arg[1]) != NULL);
}

static bool isIntegerOption(char *arg) {
    return (strchr("ghGs", arg[1]) != NULL);
}

static bool isKnownOption(char *arg) {
    return (strchr("ghGvaqcs", arg[1]) != NULL);
}

static bool isOption(char *arg) {
    return (arg[0] == '-');
}

static void print_error_exclusive_option() {
    printf("ping: Packet size and ping sweep are mutually exclusive\n");
}

static void print_error_grouped_option() {
    printf("ping: options -h -g -G must be used together\n");
}

static void print_error_argument_required(char opt) {
    printf("ping: option requires an argument -- %c\n", opt);
}

static void print_error_packet_count(char *str) {
    printf("ping: invalid count of packets to transmit: `%s'\n", str);
}

static void print_error_packet_size(char *str) {
    printf("ping: invalid packet size: `%s'\n", str);
}

static void print_error_option(char opt) {
    printf("ping: invalid option -- %c\n", opt);
}

static void print_error_usage() {
    printf("usage: ping\n");
    printf("\tsudo\n");
    printf("\t[-g sweepminsize] [-G sweepmaxsize] [-h sweepincrsize]\n");
    printf("\t[-v verbose]\n");
    printf("\t[-s packetsize]\n");
    printf("\t[-c count]\n");
    printf("\t[-a audible]\n");
    printf("\t[-q quietmode]\n");
}

static void add_grouped_option(char *opt, t_error *error) {
    if (opt[1] == 'g') {
        ++error->g;
    } else if (opt[1] == 'h') {
        ++error->h;
    } else if (opt[1] == 'G'){
        ++error->G;
    } else if (opt[1] == 's') {
        ++error->s;
    }
}

static void check_exclusive_option(t_error *error) {
    if ((error->G > 0 && error->s > 0)) {
        print_error_exclusive_option();
        exit(EXIT_FAILURE);
    }
}

static void check_grouped_option(t_error *error) {
    if ((error->g == 0 && error->h == 0 && error->G == 0) || (error->G > 0))
        return ;
    print_error_grouped_option();
    exit(EXIT_FAILURE);
}

static void check_option_format(char *opt) {
    if (strlen(opt) != 2) {
        print_error_usage();
        exit(EXIT_FAILURE);
    }
    if (isKnownOption(opt) == false) {
        print_error_option(opt[1]);
        print_error_usage();
        exit(EXIT_FAILURE);
    }
}

static void check_numeric_option_value(char **argv, t_error *error) {
    int i = 1;
    while (argv[i]) {
        if(isOption(argv[i])) {
            if (isIntegerOption(argv[i])) {
                int value = atoi(argv[i + 1]);
                if (argv[i][1] == 'g') {
                    error->g_value = value;
                } else if (argv[i][1] == 'G') {
                    error->G_value = value;
                } else if (argv[i][1] == 'h') {
                    error->h_value = value;
                } else if (argv[i][1] == 's') {
                    error->s_value = value;
                }
                ++i;
            } else if (isPositiveIntegerOption(argv[i])) {
                int value = atoi(argv[i + 1]);
                if (value <= 0) {
                    print_error_packet_count(argv[i + 1]);
                    exit(EXIT_FAILURE);
                }
            }
        }
        ++i;
    }
    if (error->h_value == 0) {
        printf("ping: invalid increment size: `0'\n");
        exit(EXIT_FAILURE);
    } else if (error->g_value >= error->G_value) {
        printf("ping: Maximum packet size must be greater than the minimum packet size\n");
        exit(EXIT_FAILURE);
    } else if (error->G_value > 65507) {
        printf("ping: packet size too large: %d > 65507\n", error->G_value);
        exit(EXIT_FAILURE);
    } else if (error->s_value > 65507) {
        printf("ping: packet size too large: %d > 65507\n", error->s_value);
        exit(EXIT_FAILURE);
    }
}

static void check_options(char **argv, t_error * error) {
    int i = 1;
    while (argv[i]) {
        if(isOption(argv[i])) {
            check_option_format(argv[i]);
            if (isIntegerOption(argv[i])) {
                add_grouped_option(argv[i], error);
                ++i;
                if (argv[i] == NULL) {
                    print_error_argument_required(argv[i - 1][1]);
                    print_error_usage();
                    exit(EXIT_FAILURE);
                }
                if (is_integer(argv[i]) == false) {
                    print_error_packet_size(argv[i]);
                    exit(EXIT_FAILURE);
                }
            } else if(isPositiveIntegerOption(argv[i])) {
                ++i;
                if (argv[i] == NULL) {
                    print_error_argument_required(argv[i - 1][1]);
                    print_error_usage();
                    exit(EXIT_FAILURE);
                }
                if (is_positive_integer(argv[i]) == false) {
                    print_error_packet_size(argv[i]);
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            ++error->nb_target;
        }
        ++i;
    }
    check_grouped_option(error);
    check_exclusive_option(error);
    check_numeric_option_value(argv, error);
}

static void check_args(t_error *error) {
    if (error->nb_target != 1) {
        print_error_usage();
        exit(EXIT_FAILURE);
    }
}

static void check_nb_args(int argc) {
    if (argc == 1) {
        print_error_usage();
        exit(EXIT_FAILURE);
    }
}

static void ini_error(t_error * error) {
    error->nb_target = 0;
    error->g = 0;
    error->G = 0;
    error->h = 0;
    error->s = 0;
    error->g_value = 0;
    error->G_value = 1;
    error->h_value = 1;
}

void check_error(int argc, char **argv) {
    t_error error;
    ini_error(&error);
    check_nb_args(argc);
    check_options(argv, &error);
    check_args(&error);
}