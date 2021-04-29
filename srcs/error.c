#include "./../ft_ping.h"

int nb_target = 0;
int g = 0;
int G = 0;
int h = 0;
int g_value = 0;
int G_value = 0;
int h_value = 0;

static bool is_numeric(char *str) {
    int i = 0;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    return (true);
}

static bool isGroupedOption(char *arg) {
    return (strchr("ghG", arg[1]) != NULL);
}

static bool isNumberOption(char *arg) {
    return (strchr("ghG", arg[1]) != NULL);
}

static bool isKnownOption(char *arg) {
    return (strchr("ghGvaq", arg[1]) != NULL);
}

static bool isOption(char *arg) {
    return (arg[0] == '-');
}

static void print_error_grouped_option() {
    printf("ping: options -h -g -G must be used together\n");
}

static void print_error_argument_required(char opt) {
    printf("ping: option requires an argument -- %c\n", opt);
}

static void print_error_packet_size(char *str) {
    printf("ping: invalid packet size: `%s'\n", str);
}

static void print_error_option(char opt) {
    printf("ping: invalid option -- %c\n", opt);
}

static void print_error_usage() {
    printf("usage: ping\n");
    printf("\t[-g sweepminsize] [-G sweepmaxsize] [-h sweepincrsize]\n");
    printf("\t[-v verbose]\n");
}

static void add_grouped_option(char *opt) {
    if (opt[1] == 'g') {
        ++g;
    } else if (opt[1] == 'h') {
        ++h;
    } else if (opt[1] == 'G'){
        ++G;
    }
}

static void check_grouped_option() {
    if ((g == 0 && h == 0 && G == 0) || (g > 0 && h > 0 && G > 0))
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

static void check_numeric_option_value(char **argv) {
    if (g == 0 && h == 0 && G == 0)
        return ;
    int i = 1;
    while (argv[i]) {
        if(isOption(argv[i])) {
            if (isNumberOption(argv[i])) {
                int value = atoi(argv[i + 1]);
                if (argv[i][1] == 'g') {
                    g_value = value;
                } else if (argv[i][1] == 'G') {
                    G_value = value;
                } else if (argv[i][1] == 'h') {
                    h_value = value;
                }
                ++i;
            }
        }
        ++i;
    }
    if (h_value == 0) {
        printf("ping: invalid increment size: `0'\n");
        exit(EXIT_FAILURE);
    } else if (g_value >= G_value) {
        printf("ping: Maximum packet size must be greater than the minimum packet size\n");
        exit(EXIT_FAILURE);
    }
}

static void check_options(char **argv) {
    int i = 1;
    while (argv[i]) {
        if(isOption(argv[i])) {
            check_option_format(argv[i]);
            if (isNumberOption(argv[i])) {
                add_grouped_option(argv[i]);
                ++i;
                if (argv[i] == NULL) {
                    print_error_argument_required(argv[i - 1][1]);
                    print_error_usage();
                    exit(EXIT_FAILURE);
                }
                if (is_numeric(argv[i]) == false) {
                    print_error_packet_size(argv[i]);
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            ++nb_target;
        }
        ++i;
    }
    check_grouped_option();
    check_numeric_option_value(argv);
}

static void check_args() {
    if (nb_target != 1) {
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

void check_error(int argc, char **argv) {
    check_nb_args(argc);
    check_options(argv);
    check_args();
}