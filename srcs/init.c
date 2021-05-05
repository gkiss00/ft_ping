#include "./../ft_ping.h"

void init_socket(t_data *data) {
    data->res = malloc(sizeof(struct addrinfo));
    data->hints.ai_flags = 0;
    data->hints.ai_family = AF_UNSPEC; //either IPV4 or IPV6
    data->hints.ai_socktype = SOCK_RAW;
    data->hints.ai_protocol = IPPROTO_ICMPV6 & IPPROTO_ICMP;
    data->hints.ai_addrlen = 0;
    data->hints.ai_addr = NULL;
    data->hints.ai_canonname = NULL;
    data->hints.ai_next = NULL;
    
    int errcode = getaddrinfo ((char*)data->target, NULL, &data->hints, &data->res);
    if (errcode < 0) {
        printf("Can't not get addrinfo\n");
        exit(EXIT_FAILURE);
    }
    while(data->res) {
        inet_ntop (data->res->ai_family, data->res->ai_addr->sa_data, data->address, 100);
        switch(data->res->ai_family) {
            case AF_INET:
                data->ptr = data->res->ai_addr;
                data->type = AF_INET;
                break;
            case AF_INET6:
                data->ptr = data->res->ai_addr;
                data->type = AF_INET6;
                break;
        }
        if (data->type == AF_INET) {
            inet_ntop (data->res->ai_family, &((struct sockaddr_in *)data->ptr)->sin_addr, data->address, 100);
        } else {
            inet_ntop (data->res->ai_family, &((struct sockaddr_in6 *)data->ptr)->sin6_addr, data->address, 100);
        }
        
        //printf ("IPv%d address: %s (%s)\n", data->res->ai_family == PF_INET6 ? 6 : 4, data->address, data->res->ai_canonname);
        data->res = data->res->ai_next;
    }
    
    if (data->type == AF_INET6) {
        data->fd = socket(data->type, SOCK_RAW, IPPROTO_ICMPV6);
    } else {
        data->fd = socket(data->type, SOCK_RAW, IPPROTO_ICMP);
    }
    if (data->fd < 0) {
        printf("ping: cannot resolve %s: Unknown host", (char*)data->target);
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(data->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) < 0) {
        printf("Can't set sockopts: reuse address");
        exit(EXIT_FAILURE);
    }
    if (data->type == AF_INET6) {
        if (setsockopt(data->fd, IPPROTO_IPV6, IP_TTL, &data->ttl, sizeof data->ttl) < 0) {
            printf("Can't set sockopts: nb ttl: %d : %s", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if (setsockopt(data->fd, IPPROTO_IP, IP_TTL, &data->ttl, sizeof data->ttl) < 0) {
            printf("Can't set sockopts: nb ttl: %d : %s", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if (setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, &data->timeout, sizeof data->timeout) < 0) {
        printf("Can't set sockopts timeout");
        exit(EXIT_FAILURE);
    }
}

// void init_socket(t_data *data) {
//     data->res = malloc(sizeof(struct addrinfo));
//     data->hints.ai_flags = 0;
//     data->hints.ai_family = AF_UNSPEC; //either IPV4 or IPV6
//     data->hints.ai_socktype = SOCK_RAW;
//     //data->hints.ai_protocol = IPPROTO_ICMP;
//     data->hints.ai_addrlen = 0;
//     data->hints.ai_addr = NULL;
//     data->hints.ai_canonname = NULL;
//     data->hints.ai_next = NULL;
    
//     int errcode = getaddrinfo ((char*)data->target, NULL, &data->hints, &data->res);
//     if (errcode < 0) {
//         printf("Can't not get addrinfo\n");
//         exit(EXIT_FAILURE);
//     }
//     while(data->res) {
//         inet_ntop (data->res->ai_family, data->res->ai_addr->sa_data, data->address, 100);
//         switch(data->res->ai_family) {
//             case AF_INET:
//                 data->ptr = data->res->ai_addr;
//                 data->type = AF_INET;
//                 break;
//             case AF_INET6:
//                 data->ptr = data->res->ai_addr;
//                 data->type = AF_INET6;
//                 break;
//         }
//         if (data->type == AF_INET) {
//             inet_ntop (data->res->ai_family, &((struct sockaddr_in *)data->ptr)->sin_addr, data->address, 100);
//         } else {
//             inet_ntop (data->res->ai_family, &((struct sockaddr_in6 *)data->ptr)->sin6_addr, data->address, 100);
//         }
        
//         printf ("IPv%d address: %s (%s)\n", data->res->ai_family == PF_INET6 ? 6 : 4, data->address, data->res->ai_canonname);
//         data->res = data->res->ai_next;
//     }
    
//     if (data->type == AF_INET6) {
//         data->fd = socket(data->type, SOCK_RAW, IPPROTO_ICMPV6);
//     } else 
//         data->fd = socket(data->type, SOCK_RAW, IPPROTO_ICMP);

//     if (data->fd < 0) {
//         printf("ping: cannot resolve %s: Unknown host", (char*)data->target);
//         exit(EXIT_FAILURE);
//     }

//     int option = 1;
//     if (setsockopt(data->fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) < 0) {
//         printf("Can't set sockopts: reuse address");
//         exit(EXIT_FAILURE);
//     }
//     if (data->type == AF_INET6) {
//         if (setsockopt(data->fd, IPPROTO_IPV6, IP_TTL, &data->ttl, sizeof data->ttl) < 0) {
//             printf("Can't set sockopts: nb ttl: %d : %s", errno, strerror(errno));
//             exit(EXIT_FAILURE);
//         }
//     } else {
//         if (setsockopt(data->fd, IPPROTO_IP, IP_TTL, &data->ttl, sizeof data->ttl) < 0) {
//             printf("Can't set sockopts: nb ttl: %d : %s", errno, strerror(errno));
//             exit(EXIT_FAILURE);
//         }
//     }
//     if (setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, &data->timeout, sizeof data->timeout) < 0) {
//         printf("Can't set sockopts timeout");
//         exit(EXIT_FAILURE);
//     }
// }

void get_nb_ping(t_data * data) {
    if (data->sweep) {
        if (data->opts.h < 0) {
            data->nb_ping = -5;
        } else {
           data->nb_ping =  (int)((int)(data->opts.G - data->opts.g) / data->opts.h) + 1;
        }
        if (data->opts.c != 0)
            data->nb_ping *= data->opts.c;
    } else {
        if (data->opts.c != 0)
            data->nb_ping = data->opts.c;
        else
            data->nb_ping = -5;
    }
}

void init_data(t_data * data) {
    data->node = NULL;
    data->target = NULL;
    data->type = AF_INET6;
    data->min = 1000000;
    data->max = -1;
    data->sum = 0;
    data->fd = 0;
    data->ttl = 118;
    data->sweep = false;
    data->nb_packet_sended = 0;
    data->nb_packet_received = 0;
    data->nb_ping = 0;
    data->opts.G = -1;
    data->opts.h = 1;
    data->opts.v = false;
    data->opts.g = 0;
    data->opts.s = 56;
    data->opts.q = false;
    data->opts.a = false;
    data->opts.t = 0;
    data->opts.c = 0;
    data->timeout.tv_sec = 1;
    data->timeout.tv_usec = 100;
}