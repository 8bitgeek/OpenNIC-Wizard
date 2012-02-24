/*
 * Copyright (c) 2002,2003 by Sergey Lyubka
 * All rights reserved
 *
 * Linked list interface.
 * This file is a part of snoopy network monitor.
 *
 * DESCRIPTION:
 * ------------
 * 
 * If you want your data structure to be organized in the list,
 * add the `struct llhead' node in it. By doing this, the data structure
 * will be organized in a circular doubly-linked list. If you want the data
 * structure to be presented in more than onelinked list, add the
 * appropriate number of `struct llhead' nodes.
 *
 * Each `struct llhead' node must be initialized before use: the initialized
 * node have both `next' and `prev' pointers pointed by itself, marking that
 * the list is empty, or, in other words, the data structure is not linked to
 * any list yet. The initialization is done by macro LL_INIT(ptr)
 * 
 * If you declare a list head as a static variable, you may define it as
 * LL_HEAD(your_list_head) so it will be initialized, and there is no need
 * to call LL_INIT(&your_list_head) later.
 *
 * Use macros LL_FOREACH(head, ptr) and LL_FOREACH_SAFE(head, ptr, temp_ptr)
 * to loop over the list. The latter is used if the elements may be removed
 * while looping over the list. Both these macros set the variable `ptr'
 * which has type `struct llhead *'. Likely you will want to have a pointer
 * to the data structure that holds the `struct llhead' node, and that pointer
 * can be obtained by LL_ENTRY(ptr, struct_type, node_name) macro.
 * The first parameter is the pointer to `struct llhead' node,
 * the second one is the name of the data structure that includes the node,
 * and the third one is the name of the node in the data structure.
 *
 * USAGE EXAMPLE:
 * --------------
 * 
 * You have a structure that describes the network host
 * struct host {
 * 	struct in_addr host_ip;	
 * 	char *host_name;
 * };
 *
 * Now, you need to organize the hosts in the list. First, you need
 * to modify struct host and add the llhead node in it:
 * struct host {
 * 	struct in_addr host_ip;
 * 	char *host_name;
 * 	struct llhead host_list;
 * };
 *
 * Then, somewhere in the code should be declared a head of the host list:
 * static LL_HEAD(host_list_head);
 *
 * Now, the example function that allocates a new host and adds it to the list:
 * struct host *newhost(const struct in_addr *ip, const char *name)
 * {
 * 	struct host *hp;
 *
 * 	if ((hp = malloc(sizeof(*hp))) != NULL) {
 * 		hp->host_ip = *ip;
 * 		hp->host_name = strdup(name);
 * 		LL_ADD(&host_list_head, &hp->host_list);
 * 	}
 *
 * 	return (hp);
 * }
 *
 * The example code that loops through the all hosts and prints its names:
 * struct llhead *lp;
 * struct host *hp;
 * LL_FOREACH(&host_list_head, lp) {
 * 	hp = LL_ENTRY(lp, struct host, host_list);
 * 	printf("%s\n", hp->host_name);
 * }
 *
 * The example code that loops through the all hosts and deallocates them:
 * struct llhead *lp, *tmp;
 * struct host *hp;
 * LL_FOREACH_SAFE(&host_list_head, lp, tmp) {
 * 	hp = LL_ENTRY(lp, struct host, host_list);
 * 	free(hp->host_name);
 * 	free(hp);
 * }
 *
 *
 * REFERENCE:
 * ----------
 *
 *  LL_INIT:  initializes the list
 *  LL_ENTRY: convert list pointer to the pointer to struct that
 *  	contains that list. t - type of a struct, p - ptr to list head,
 *  	n - name of a list node in a struct
 *  LL_ADD: add an entry to the list, stack implementation
 *  LL_TAIL: add an entry to the list, queue implementation
 *  LL_DEL: remove an entry from the list
 *  LL_FOREACH: loop over all elements in a list. h - head, p - iterator
 *  LL_FOREACH_SAFE: loop over all elements in a list. h - head,
 *  	p - iterator, tmp - temporary storage
 *  	this macro is safe to use for the list element removal.
 *  LL_EMPTY: nonzero if the list is empty, 0 otherwise
 *
 * $Id: llist.h,v 1.1 2005/11/15 21:58:04 drozd Exp $
 */

#ifndef LLIST_HEADER_INCLUDED
#define LLIST_HEADER_INCLUDED

#include <stddef.h>		/* for offsetof*/

struct llhead {
	struct llhead *prev, *next;
};

#define	LL_INIT(N)	((N)->next = (N)->prev = (N))

#define LL_HEAD(H)	struct llhead H = { &H, &H }

#define	LL_ENTRY(P,T,N)	((T *) ((char *) (P) - offsetof(T,N)))

#define	LL_ADD(H, N)							\
	do {								\
		((H)->next)->prev = (N);				\
		(N)->next = ((H)->next);				\
		(N)->prev = (H);					\
		(H)->next = (N);					\
	} while (0)

#define	LL_TAIL(H, N)							\
	do {								\
		((H)->prev)->next = (N);				\
		(N)->prev = ((H)->prev);				\
		(N)->next = (H);					\
		(H)->prev = (N);					\
	} while (0)

#define	LL_DEL(N)							\
	do {								\
		((N)->next)->prev = ((N)->prev);			\
		((N)->prev)->next = ((N)->next);			\
		LL_INIT(N);						\
	} while (0)

#define	LL_EMPTY(N)	((N)->next == (N))

#define	LL_FOREACH(H,N)	for (N = (H)->next; N != (H); N = (N)->next)

#define LL_FOREACH_SAFE(H,N,T)						\
	for (N = (H)->next, T = (N)->next; N != (H);			\
			N = (T), T = (N)->next)

#endif /* LLIST_HEADER_INCLUDED */
