# include "./ft_ping.h"

t_data data;
t_option option;

int                 fd = 0, nb_packet_sended = 0, nb_packed_received = 0;
double              min = 1000, max = -1, sumary = 0, avg, stddev;
void                *ptr;
struct addrinfo     hints;
struct addrinfo     *res;
char                addrstr[100];
uint32_t            type;
int ttl = 118; //nb max of router before getting destroyed
char *domain;
struct timeval timing;

uint16_t    checksum(uint16_t *msg, uint32_t size) {
    uint32_t sum = 0;
    uint16_t check = 0;
    for (uint32_t i = 0; i < size; ++i) {
        sum += msg[i];
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
    double mean = mean;
    double s = 0;
    double sum = 0;
    double count = 0;
    while(tmp) {
        sum += pow(tmp->time - mean, 2);
        ++count;
        tmp = tmp->next;
    }
    return count == 0 ? 0 : sqrt(sum / count);
}

static void end(int signal) {
    double pct = 1 - (double)((double)nb_packed_received / (double)nb_packet_sended);
    pct *= 100;
    printf("\n--- %s ping statistics ---\n", domain);
    printf("%d packets transmitted, %d packets received, %.01f%% packet loss\n", nb_packet_sended, nb_packed_received, pct);
    if (nb_packed_received > 0)
        printf("round-trip min/avg/max/stddev = %.03f/%.03f/%.03f/%.03f ms\n", min, sumary / nb_packet_sended, max, S());
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
        if (errno == EHOSTUNREACH) {
            printf("ping: sendto: %s\n", strerror(errno));
        } else {
            printf("Error while sending package: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
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
            if (errno == EWOULDBLOCK){
                if (data.opts->q == false) {
                    printf("Request timeout for icmp_seq %d\n", nb_packet_sended - 1);
                }
            }
                //printf("%d : Error %d while receiving package: %s\n", z, errno, strerror(errno));
            //exit(EXIT_FAILURE);
            errno = 0;
        } else {
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
                node_add_back(&data.node, new_node(diff));
                if (data.opts->q == false) {
                    if (data.opts->a)
                        printf("\a");
                    printf("64 bytes from %s: icmp_seq=%d ttl=%d time=%.03f ms\n", addrstr, nb_packet_sended - 1, ttl, diff);
                }
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
    
    getaddrinfo((char*)data.target, NULL, &hints, &res);
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
            inet_ntop (res->ai_family, &((struct sockaddr_in6 *)ptr)->sin6_addr, addrstr, 100);
        }
        
        printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        res = res->ai_next;
    }
    
    fd = socket(type, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0) {
        printf("Error while init socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    
    setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof ttl);
    timing.tv_sec = 1;
    timing.tv_usec = 100;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timing, sizeof timing);
}

static void init_data(t_data * data, t_option *opt) {
    data->node = NULL;
    data->target = NULL;
    data->opts = opt;
    data->opts->G = -1;
    data->opts->h = -1;
    data->opts->v = -1;
    data->opts->g = -1;
    data->opts->q = false;
    data->opts->a = false;
}

int     main(int argc, char **argv) {

    init_data(&data, &option);
    check_error(argc, argv);
    parsing(&data, (uint8_t**)argv);
    
    domain = argv[1];
    init_socket(argv);
    printf("PING %s (%s): 56 data bytes\n", argv[1], addrstr);
    
    signal(SIGINT, end);
    signal(SIGALRM, ping_penguin);

    send_ping();
    alarm(1);
    receive_ping();
}