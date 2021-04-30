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

//average of time for all responding ping
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

// standar deviation for all responging ping
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

// get size of the current package
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

// check if the size of the package is valid
static bool canBeSend(int size) {
    if (size < HEADER_SIZE) {
        printf("ping: sendto: Invalid argument\n");
        return false;
    }
    return true;    
}

//did all package get sended 
static void checkForEnd() {
    if (data.nb_packet_sended == data.nb_ping) {
        end(1);
    }
}

static void send_ping() {
    ++data.nb_packet_sended;
    checkForEnd(); // is the right number of ping get sended
    int     size = getSize();

    if(!canBeSend(size)) // is the size of the packet bigger or equal 8
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

    gettimeofday(&data.sending_time, NULL); // stock the sending time
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
        int z = recvmsg(data.fd, &msg, 0); // try to receive a message before time out
        if (z < 0) {
            print_receiving_error();
        } else {
            struct icmp *response;   
            response = (struct icmp *)&msg_buffer[20];
            //printf("%d : %d\n", response->icmp_id,  getpid());
            if (response->icmp_type == ICMP_ECHOREPLY && response->icmp_id == getpid()) {
                gettimeofday(&data.receiving_time, NULL); // stock the receiving time
                double diff = ((data.receiving_time.tv_sec - data.sending_time.tv_sec) * UINT32_MAX + (data.receiving_time.tv_usec - data.sending_time.tv_usec)) / 1000;
                data.sum += diff;
                data.min = data.min > diff ? (diff) : (data.min);
                data.max = data.max < diff ? (diff) : (data.max);
                ++data.nb_packet_received;
                node_add_back(&data.node, new_node(diff)); // stock the new data
                print_good(diff, z - 20);
            } else {
                //printf("shit\n");
            }
        }
    }
}

static void ping_penguin(int sig) { //each second send a ping
    sig = 0;
    send_ping();
    signal(SIGALRM, ping_penguin);
    alarm(1);
}

int     main(int argc, char **argv) {
    init_data(&data);
    check_error(argc, argv);
    parsing(&data, (uint8_t**)argv);
    get_nb_ping(&data);
    init_socket(&data);
    begin();
    
    signal(SIGINT, end);
    signal(SIGALRM, ping_penguin);

    send_ping();
    alarm(1);
    receive_ping();
}