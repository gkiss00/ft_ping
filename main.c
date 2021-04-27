# include "./ft_ping.h"

int  fd = 0, nb_packet_sended, nb_packed_received;
void                *ptr;
struct addrinfo     hints;
struct addrinfo     *res;
char                addrstr[100];
uint32_t            type;

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

}

static void send_ping() {
    uint8_t buff[ICMP_SIZE + TIME_SIZE];

    // SIZE = 28
    struct icmp test;
    // SIZE = 16    
    struct timeval tv;
    // SIZE = 8
    struct timezone tz;

    test.icmp_type = 8;
    test.icmp_code = 0;
    test.icmp_cksum = 0;
    test.icmp_hun.ih_idseq.icd_id = getpid();
    test.icmp_hun.ih_idseq.icd_seq = nb_packet_sended;
    ++nb_packet_sended;

    gettimeofday(&tv, &tz);

    memcpy(buff, &test, ICMP_SIZE);
    memcpy(&buff[ICMP_SIZE], &tv, TIME_SIZE);
    test.icmp_cksum = checksum((uint16_t *)buff, sizeof(buff) / 2);
    memcpy(buff, &test, ICMP_SIZE);

    int x = sendto(fd, buff, ICMP_SIZE + TIME_SIZE, 0, (struct sockaddr*)ptr, sizeof(struct sockaddr));
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
    msg.msg_iovlen = 0; // len of msg_iov (size_t)
    msg.msg_control = NULL; // metadata (void *)
    msg.msg_controllen = 0; //size of ... (socklen_t)
    msg.msg_flags = 0; // atributes of msg received (int);

    int z = recvmsg(fd, &msg, 0);
    if (z < 0) {
        printf("Error %d while receiving package: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void pingou_pingouin(int signal) {
    signal = 0;
    send_ping();
    //signal(SIGALRM, pingou_pingouin);
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

    int ttl = 118; //nb max of touterbefore getting destroyed
    setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof ttl);
}

int     main(int argc, char **argv) {
    if (argc == 1)
        exit(EXIT_FAILURE);

    init_socket(argv);
    send_ping();
    receive_ping();

    exit(EXIT_SUCCESS);

    signal(SIGQUIT, end);
    signal(SIGALRM, pingou_pingouin);
}