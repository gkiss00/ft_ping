# include "./ft_ping.h"

int     main(int argc, char **argv) {
    if (argc == 1)
        exit(EXIT_FAILURE);

    struct addrinfo     hints;
    struct addrinfo     *res;
    struct msghdr       *msg;
    void                *ptr;
    char                addrstr[100];

    res = malloc(sizeof(struct addrinfo));
    msg = malloc(sizeof(struct msghdr));
    
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
                break;
            case AF_INET6:
                ptr = res->ai_addr;
                break;
        }
        inet_ntop (res->ai_family, &((struct sockaddr_in *)ptr)->sin_addr, addrstr, 100);
        printf ("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        res = res->ai_next;
    }
    
    int fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0) {
        printf("Error while init socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int ttl = 118; //nb max of touterbefore getting destroyed
    setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof ttl);

    char *buff = "123";
    printf("%p\n", buff);
    printf("%p\n", &buff);
    printf("%d\n", sizeof(struct sockaddr));
    int x = sendto(fd, buff, 3, 0, (struct sockaddr*)ptr, sizeof(struct sockaddr));
    if (x < 0) {
        printf("Error while sending package: %s\n", strerror(errno));
    }
    printf("%d\n", x);
    struct iovec iov[1];
    char    msg_buffer[256];
    iov[0].iov_base = msg_buffer;     
    iov[0].iov_len = sizeof msg_buffer;
    msg->msg_name = NULL; //optional address (void*)
    msg->msg_namelen = 0; // address size (socklen_t)
    msg->msg_iov = iov; //table scatter/gather (struct iovec)
    msg->msg_iovlen = 0; // len of msg_iov (size_t)
    msg->msg_control = NULL; // metadata (void *)
    msg->msg_controllen = 0; //size of ... (socklen_t)
    msg->msg_flags = 0; // atributes of msg received (int);
    int z = recvmsg(fd, msg, MSG_WAITALL);
    if (x < 0) {
        printf("Error while receiving package: %s\n", strerror(errno));
    }
    printf("%s\n", msg_buffer);
    exit(EXIT_SUCCESS);
}