#ifndef FT_PING_H
#define FT_PING_H

#define ICMP_SIZE 28
#define TIME_SIZE 16

# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <fcntl.h>
# include <stdio.h>
# include <string.h>
# include <assert.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <errno.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <math.h>
# include <stdbool.h>


typedef struct          s_node
{
    double             time;
    struct s_node         *next;
}                       t_node;

typedef struct          s_option
{
    uint32_t             h; //sweep increment size
    uint32_t             g; //min sweep size
    uint32_t             G; //max sweep size
    uint32_t             v; //verbose output
    bool                 q; //quiet mode
    bool                 a; //bip sound
}                       t_option;

typedef struct		    s_data
{
	uint8_t             *target;
    struct s_node       *node;
    struct s_option     *opts;
}                       t_data;

t_node *new_node(double time);
t_node *node_last(t_node *node);
void node_add_back(t_node **head, t_node *new);

void check_error(int argc, char **argv);
void parsing(t_data *data, uint8_t **argv);


#endif