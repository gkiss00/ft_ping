#include "./../ft_ping.h"

t_node *new_node(double time) {
    t_node *node;

    node = malloc(sizeof(t_node));
    if(node == NULL){
        printf("Malloc error\n");
        exit(EXIT_FAILURE);
    }
    node->time = time;
    node->next = NULL;
    return (node);
}

t_node *node_last(t_node *node) {
    while (node && node->next) {
        node = node->next;
    }
    return (node);
}

void node_add_back(t_node **head, t_node *new) {
    if (head != 0)
	{
		if (*head && new != 0)
		{
			node_last(*head)->next = new;
		}
		else
		{
			*head = new;
		}
	}
}