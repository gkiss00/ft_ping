# include "./ft_ping.h"

int                 fd = 0, nb_packet_sended, nb_packed_received;
double              min = 1000, max = -1, sumary = 0, avg, stddev;
void                *ptr;
struct addrinfo     hints;
struct addrinfo     *res;
char                addrstr[100];
uint32_t            type;
int ttl = 118; //nb max of touterbefore getting destroyed
char *domain;

uint16_t    checksum(uint16_t *msg, uint32_t size) {
    uint32_t sum = 0;
    uint16_t check = 0;
    for (uint32_t i = 0; i < size; ++i) {
        sum += msg[i];
    }
    check = sum % UINT16_MAX;
    return ~check;
}

static void end(int signal) {
    double pct = 1 - (double)((double)nb_packed_received / (double)nb_packet_sended);
    pct *= 100;
    printf("--- %s ping statistics ---\n", domain);
    printf("%d packets transmitted, %d packets received, %.01f%% packet loss\n", nb_packet_sended, nb_packed_received, pct);
    printf("round-trip min/avg/max/stddev = %.03f/%.03f/%.03f/%d ms\n", min, max, sumary / nb_packet_sended, 0);
    exit(EXIT_SUCCESS);
}

static void send_ping() {
    uint8_t buff[64];
    memset(buff, 0, 64);

    // SIZE = 28
    struct icmp test;
    // SIZE = 16    
    struct timeval tv;

    test.icmp_type = 8;
    test.icmp_code = 0;
    test.icmp_cksum = 0;
    test.icmp_id = getpid();
    test.icmp_seq = nb_packet_sended;
    ++nb_packet_sended;

    gettimeofday(&tv, NULL);

    memcpy(buff, &test, ICMP_SIZE);
    memcpy(&buff[ICMP_SIZE], &tv, TIME_SIZE);
    test.icmp_cksum = checksum((uint16_t *)buff, sizeof(buff) / 2);
    memcpy(buff, &test, ICMP_SIZE);

    int x = sendto(fd, buff, 64, 0, (struct sockaddr*)ptr, sizeof(struct sockaddr));
    if (x < 0) {
        printf("Error while sending package: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
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
        int z = recvmsg(fd, &msg, 0);
        if (z < 0) {
            printf("Error %d while receiving package: %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }

        struct icmp *response;   
        response = (struct icmp *)&msg_buffer[20];

        if (response->icmp_type == ICMP_ECHOREPLY && response->icmp_id == getpid()) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            struct timeval *tv2 = (struct timeval *)&msg_buffer[20 + ICMP_SIZE];
            double diff = (tv.tv_sec - tv2->tv_sec) * UINT32_MAX + (tv.tv_usec - tv2->tv_usec);
            diff /= 1000;
            sumary += diff;
            if (min > diff)
                min = diff;
            if (max < diff)
                max = diff;
            ++nb_packed_received;
            printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.03f ms\n", addrstr, nb_packet_sended - 1, ttl, diff);
        } else {
            printf("SHIT\n");
        }
    }
}

static void pingou_pingouin(int sig) {
    sig = 0;
    send_ping();
    signal(SIGALRM, pingou_pingouin);
    alarm(1);
}

static void init_socket(char **argv) {
    res = malloc(sizeof(struct addrinfo));
    
    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC; //either IPV4 or IPV6
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    hints.ai_addrlen = 0;
    hints.ai_addr = NULL;
    hints.ai_canonname = NULL;
    hints.ai_next = NULL;
    
    getaddrinfo(argv[1], NULL, &hints, &res);
    while(res) {
        inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);
        switch(res->ai_family) {
            case AF_INET:
                ptr = res->ai_addr;
                type = AF_INET;
                break;
            case AF_INET6:
                ptr = res->ai_addr;
                type = AF_INET6;
                break;
        }
        if (type == AF_INET) {
            inet_ntop (res->ai_family, &((struct sockaddr_in *)ptr)->sin_addr, addrstr, 100);
        } else {
            inet_ntop (res->ai_family, &((struct sockaddr_in *)ptr)->sin_addr, addrstr, 100);
        }
        
        printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        res = res->ai_next;
    }
    
    fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0) {
        printf("Error while init socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    
    setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof ttl);
}

int     main(int argc, char **argv) {
    if (argc == 1)
        exit(EXIT_FAILURE);

    domain = argv[1];
    init_socket(argv);
    printf("PING %s (%s): 56 data bytes\n", argv[1], addrstr);
    
    signal(SIGINT, end);
    signal(SIGALRM, pingou_pingouin);

    send_ping();
    alarm(1);
    receive_ping();
}