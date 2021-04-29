#include "./../ft_ping.h"

bool is_positive_float(char *str) {
    int i = 0;
    bool sign = false;
    if(str[i] == '+') {
        ++i;
        sign = true;
    }
    if (i == 1 && strlen(str) == 1)
        return false;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    if ((sign && i == 1) || (!sign && i == 0))
        return false;
    if(str[i] == '.')
        ++i;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    return (true);
}

bool is_float(char *str) {
    int i = 0;
    bool sign = false;
    if(str[i] == '-' || str[i] == '+') {
        ++i;
        sign = true;
    }
    if (i == 1 && strlen(str) == 1)
        return false;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    if ((sign && i == 1) || (!sign && i == 0))
        return false;
    if(str[i] == '.')
        ++i;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    return (true);
}

bool is_positive_integer(char *str) {
    int i = 0;
    if(str[i] == '+')
        ++i;
    if (i == 1 && strlen(str) == 1)
        return false;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    return (true);
}

bool is_integer(char *str) {
    int i = 0;
    if(str[i] == '-' || str[i] == '+')
        ++i;
    if (i == 1 && strlen(str) == 1)
        return false;
    while(str[i]) {
        if (str[i] < '0' || str[i] > '9')
            return (false);
        ++i;
    }
    return (true);
}