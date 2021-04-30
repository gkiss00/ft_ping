# include "./ft_ping.h"

t_data data;

uint16_t    checksum(uint8_t *msg, uint32_t size) {
    uint32_t    new_size = size % 2 == 0 ? size : size + 1;
    uint16_t    tmp[new_size / 2];
    memset(tmp, 0, new_size);
    memcpy(tmp, msg, size);
    uint32_t sum = 0;
    uint16_t check = 0;
    for (uint32_t i = 0; i < new_size / 2; ++i) {
        sum += tmp[i];
    }
    check = sum % UINT16_MAX;
    return ~check;
}

static double mean() {
    t_node *tmp = data.node;
    double sum = 0;
    double count = 0;


    while(tmp) {
        sum += tmp->time;
        ++count;
        tmp = tmp->next;
    }
    return count == 0 ? 0 : (sum / count);
}

static double S() {
    t_node *tmp = data.node;
    double average = mean();
    double sum = 0;
    double count = 0;
    while(tmp) {
        sum += pow(tmp->time - average, 2);
        ++count;
        tmp = tmp->next;
    }
    return count == 0 ? 0 : sqrt(sum / count);
}

static void end(int signal) {
    signal = 0;
    double pct = 1 - (double)((double)data.nb_packet_received / (double)data.nb_packet_sended);
    pct *= 100;
    printf("\n--- %s ping statistics ---\n", data.target);
    printf("%d packets transmitted, %d packets received, %.01f%% packet loss\n", data.nb_packet_sended, data.nb_packet_received, pct);
    if (data.nb_packet_received > 0)
        printf("round-trip min/avg/max/stddev = %.03f/%.03f/%.03f/%.03f ms\n", data.min, data.sum / data.nb_packet_sended, data.max, S());
    exit(EXIT_SUCCESS);
}

static void begin() {
    if (data.sweep) {
        printf("PING %s (%s): (%d ... %d) data bytes\n", data.target, data.address, data.opts.g, data.opts.G); 
    } else {
        printf("PING %s (%s): %d data bytes\n", data.target, data.address, data.opts.s);
    }
}

static void print_good(double diff, int size) {
    if (data.opts.q == false) {
        if (data.opts.a)
            printf("\a");
        if (size < 24)
            printf("%d bytes from %s: icmp_seq=%d ttl=%d\n", size, data.address, data.nb_packet_sended, 64);
        else
            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.03f ms\n", size, data.address, data.nb_packet_sended, 64, diff);
    }
}

static void print_receiving_error() {
    if (errno == EWOULDBLOCK){
        if (data.opts.q == false) {
            printf("Request timeout for icmp_seq %d\n", data.nb_packet_sended);
        }
    }
}

static void print_sending_error() {
    if (errno == EHOSTUNREACH) {
        printf("ping: sendto: %s\n", strerror(errno));
    } else {
        printf("Error while sending package: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static int getSize() {
    int     size = HEADER_SIZE + data.opts.s;
    if (data.sweep) {
        if (data.opts.c != 0) {
            int x = data.nb_packet_sended / data.opts.c;
            size = data.opts.g + (x * data.opts.h) + HEADER_SIZE;
        } else {
            size = data.opts.g + (data.nb_packet_sended * data.opts.h) + HEADER_SIZE;
        }
    }
    return (size);
}

static bool canBeSend(int size) {
    if (size < HEADER_SIZE) {
        printf("ping: sendto: Invalid argument\n");
        return false;
    }
    return true;    
}

static void checkForEnd() {
    if (data.nb_packet_sended == data.nb_ping) {
        end(1);
    }
}

static void send_ping() {
    ++data.nb_packet_sended;
    checkForEnd();
    int     size = getSize();

    if(!canBeSend(size))
        return;

    uint8_t buff[size];
    memset(buff, 0, size);

    // SIZE = 28
    struct icmp icmp;
    icmp.icmp_type = 8;
    icmp.icmp_code = 0;
    icmp.icmp_cksum = 0;
    icmp.icmp_id = getpid();
    icmp.icmp_seq = data.nb_packet_sended;

    memcpy(buff, &icmp, size);
    icmp.icmp_cksum = checksum(buff, size);
    memcpy(buff, &icmp, size);

    gettimeofday(&data.sending_time, NULL);
    int x = sendto(data.fd, buff, size, 0, (struct sockaddr*)data.ptr, sizeof(struct sockaddr));
    if (x < 0) {
        print_sending_error();
    }
}

static void receive_ping() {
    uint8_t    msg_buffer[256];
    memset(msg_buffer, 0, sizeof(msg_buffer));

    struct iovec iov[1];
    memset(iov, 0, sizeof(iov));
    iov[0].iov_base = msg_buffer;
    iov[0].iov_len = sizeof msg_buffer ;

    struct msghdr       msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = NULL; //optional address (void*)
    msg.msg_namelen = 0; // address size (socklen_t)
    msg.msg_iov = iov; //table scatter/gather (struct iovec)
    msg.msg_iovlen = 1; // len of msg_iov (size_t)
    msg.msg_control = NULL; // metadata (void *)
    msg.msg_controllen = 0; //size of ... (socklen_t)
    msg.msg_flags = 0; // atributes of msg received (int);

    while(1){
        int z = recvmsg(data.fd, &msg, 0);
        if (z < 0) {
            print_receiving_error();
        } else {
            struct icmp *response;   
            response = (struct icmp *)&msg_buffer[20];

            if (response->icmp_type == ICMP_ECHOREPLY && response->icmp_id == getpid()) {
                gettimeofday(&data.receiving_time, NULL);
                double diff = (data.receiving_time.tv_sec - data.sending_time.tv_sec) * UINT32_MAX + (data.receiving_time.tv_usec - data.sending_time.tv_usec);
                diff /= 1000;
                data.sum += diff;
                if (data.min > diff)
                    data.min = diff;
                if (data.max < diff)
                    data.max = diff;
                ++data.nb_packet_received;
                node_add_back(&data.node, new_node(diff));
                print_good(diff, z - 20);
            }
        }
    }
}

static void ping_penguin(int sig) {
    sig = 0;
    send_ping();
    signal(SIGALRM, ping_penguin);
    alarm(1);
}

static void init_socket() {
    data.res = malloc(sizeof(struct addrinfo));
    
    data.hints.ai_flags = 0;
    data.hints.ai_family = AF_UNSPEC; //either IPV4 or IPV6
    data.hints.ai_socktype = SOCK_RAW;
    data.hints.ai_protocol = IPPROTO_ICMP;
    data.hints.ai_addrlen = 0;
    data.hints.ai_addr = NULL;
    data.hints.ai_canonname = NULL;
    data.hints.ai_next = NULL;
    
    int errcode = getaddrinfo ((char*)data.target, NULL, &data.hints, &data.res);
    if (errcode < 0) {
        exit(EXIT_FAILURE);
    }
    while(data.res) {
        inet_ntop (data.res->ai_family, data.res->ai_addr->sa_data, data.address, 100);
        switch(data.res->ai_family) {
            case AF_INET:
                data.ptr = data.res->ai_addr;
                data.type = AF_INET;
                break;
            case AF_INET6:
                data.ptr = data.res->ai_addr;
                data.type = AF_INET6;
                break;
        }
        if (data.type == AF_INET) {
            inet_ntop (data.res->ai_family, &((struct sockaddr_in *)data.ptr)->sin_addr, data.address, 100);
        } else {
            inet_ntop (data.res->ai_family, &((struct sockaddr_in6 *)data.ptr)->sin6_addr, data.address, 100);
        }
        
        //printf ("IPv%d address: %s (%s)\n", data.res->ai_family == PF_INET6 ? 6 : 4, data.address, data.res->ai_canonname);
        data.res = data.res->ai_next;
    }
    
    data.fd = socket(data.type, SOCK_RAW, IPPROTO_ICMP);
    if (data.fd < 0) {
        printf("ping: cannot resolve %s: Unknown host", (char*)data.target);
        exit(EXIT_FAILURE);
    }

    setsockopt(data.fd, IPPROTO_IP, IP_TTL, &data.ttl, sizeof data.ttl);
    data.timeout.tv_sec = 1;
    data.timeout.tv_usec = 100;
    setsockopt(data.fd, SOL_SOCKET, SO_RCVTIMEO, &data.timeout, sizeof data.timeout);
}

static void get_nb_ping(t_data * data) {
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

static void init_data(t_data * data) {
    data->node = NULL;
    data->target = NULL;
    data->min = 1000000;
    data->max = -1;
    data->sum = 0;
    data->fd = 0;
    data->ttl = 118;
    data->sweep = false;
    data->nb_packet_sended = -1;
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
}

int     main(int argc, char **argv) {

    init_data(&data);
    check_error(argc, argv);
    parsing(&data, (uint8_t**)argv);
    get_nb_ping(&data);
    init_socket();
    begin();
    
    
    signal(SIGINT, end);
    signal(SIGALRM, ping_penguin);

    send_ping();
    alarm(1);
    receive_ping();
}