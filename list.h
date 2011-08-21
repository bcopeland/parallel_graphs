/*
 * These list routines are slightly modified versions of those from CCAN
 * which in turn came from the Linux kernel (GPL v2).
 */
#ifndef _LIST_H
#define _LIST_H

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef offsetof
#define offsetof(type, element) \
    ((unsigned long) &((type *)0)->element)
#endif


/**
 * check_type - issue a warning or build failure if type is not correct.
 * @expr: the expression whose type we should check (not evaluated).
 * @type: the exact type we expect the expression to be.
 *
 * This macro is usually used within other macros to try to ensure that a macro
 * argument is of the expected type.  No type promotion of the expression is
 * done: an unsigned int is not the same as an int!
 *
 * check_type() always evaluates to 1.
 *
 * If your compiler does not support typeof, then the best we can do is fail
 * to compile if the sizes of the types are unequal (a less complete check).
 *
 * Example:
 *	// They should always pass a 64-bit value to _set_some_value!
 *	#define set_some_value(expr)			\
 *		_set_some_value((check_type((expr), uint64_t), (expr)))
 */

/**
 * check_types_match - issue a warning or build failure if types are not same.
 * @expr1: the first expression (not evaluated).
 * @expr2: the second expression (not evaluated).
 *
 * This macro is usually used within other macros to try to ensure that
 * arguments are of identical types.  No type promotion of the expressions is
 * done: an unsigned int is not the same as an int!
 *
 * check_types_match() always evaluates to 0.
 *
 * If your compiler does not support typeof, then the best we can do is fail
 * to compile if the sizes of the types are unequal (a less complete check).
 *
 * Example:
 *	// Do subtraction to get to enclosing type, but make sure that
 *	// pointer is of correct type for that member. 
 *	#define container_of(mbr_ptr, encl_type, mbr)			\
 *		(check_types_match((mbr_ptr), &((encl_type *)0)->mbr),	\
 *		 ((encl_type *)						\
 *		  ((char *)(mbr_ptr) - offsetof(enclosing_type, mbr))))
 */
#define check_type(expr, type)			\
	((typeof(expr) *)0 != (type *)0)

#define check_types_match(expr1, expr2)		\
	((typeof(expr1) *)0 != (typeof(expr2) *)0)

/**
 * container_of - get pointer to enclosing structure
 * @member_ptr: pointer to the structure member
 * @containing_type: the type this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	struct info
 *	{
 *		int some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	struct info *foo_to_info(struct foo *foop)
 *	{
 *		return container_of(foo, struct info, my_foo);
 *	}
 */
#define container_of(member_ptr, containing_type, member)		\
	 ((containing_type *)						\
	  ((char *)(member_ptr) - offsetof(containing_type, member))	\
	  - check_types_match(*(member_ptr), ((containing_type *)0)->member))


/**
 * container_of_var - get pointer to enclosing structure using a variable
 * @member_ptr: pointer to the structure member
 * @var: a pointer to a structure of same type as this member is within
 * @member: the name of this member within the structure.
 *
 * Given a pointer to a member of a structure, this macro does pointer
 * subtraction to return the pointer to the enclosing type.
 *
 * Example:
 *	struct info
 *	{
 *		int some_other_field;
 *		struct foo my_foo;
 *	};
 *
 *	struct info *foo_to_info(struct foo *foop)
 *	{
 *		struct info *i = container_of_var(foo, i, my_foo);
 *		return i;
 *	}
 */
#define container_of_var(member_ptr, var, member) \
	container_of(member_ptr, typeof(*var), member)

/**
 * struct list_node - an entry in a doubly-linked list
 * @next: next entry (self if empty)
 * @prev: previous entry (self if empty)
 *
 * This is used as an entry in a linked list.
 * Example:
 *	struct child {
 *		const char *name;
 *		// Linked list of all us children.
 *		struct list_node list;
 *	};
 */
struct list_node
{
	struct list_node *next, *prev;
};

/**
 * struct list_head - the head of a doubly-linked list
 * @h: the list_head (containing next and prev pointers)
 *
 * This is used as the head of a linked list.
 * Example:
 *	struct parent {
 *		const char *name;
 *		struct list_head children;
 *		unsigned int num_children;
 *	};
 */
struct list_head
{
	struct list_node n;
};

/**
 * list_check - check a list for consistency
 * @h: the list_head
 * @abortstr: the location to print on aborting, or NULL.
 *
 * Because list_nodes have redundant information, consistency checking between
 * the back and forward links can be done.  This is useful as a debugging check.
 * If @abortstr is non-NULL, that will be printed in a diagnostic if the list
 * is inconsistent, and the function will abort.
 *
 * Returns the list head if the list is consistent, NULL if not (it
 * can never return NULL if @abortstr is set).
 *
 * Example:
 *	static void dump_parent(struct parent *p)
 *	{
 *		struct child *c;
 *
 *		printf("%s (%u children):\n", p->name, parent->num_children);
 *		list_check(&p->children, "bad child list");
 *		list_for_each(&p->children, c, list)
 *			printf(" -> %s\n", c->name);
 *	}
 */
struct list_head *list_check(const struct list_head *h, const char *abortstr);

#ifdef CCAN_LIST_DEBUG
#define debug_list(h) list_check((h), __func__)
#else
#define debug_list(h) (h)
#endif

/**
 * list_head_init - initialize a list_head
 * @h: the list_head to set to the empty list
 *
 * Example:
 *	list_head_init(&parent->children);
 *	parent->num_children = 0;
 */
static inline void list_head_init(struct list_head *h)
{
	h->n.next = h->n.prev = &h->n;
}

/**
 * LIST_HEAD - define and initalized empty list_head
 * @name: the name of the list.
 *
 * The LIST_HEAD macro defines a list_head and initializes it to an empty
 * list.  It can be prepended by "static" to define a static list_head.
 *
 * Example:
 *	// Header:
 *	extern struct list_head my_list;
 *
 *	// C file:
 *	LIST_HEAD(my_list);
 */
#define LIST_HEAD(name) \
	struct list_head name = { { &name.n, &name.n } }

/**
 * list_add - add an entry at the start of a linked list.
 * @h: the list_head to add the node to
 * @n: the list_node to add to the list.
 *
 * The list_node does not need to be initialized; it will be overwritten.
 * Example:
 *	list_add(&parent->children, &child->list);
 *	parent->num_children++;
 */
static inline void list_add(struct list_head *h, struct list_node *n)
{
	n->next = h->n.next;
	n->prev = &h->n;
	h->n.next->prev = n;
	h->n.next = n;
	(void)debug_list(h);
}

/**
 * list_add_tail - add an entry at the end of a linked list.
 * @h: the list_head to add the node to
 * @n: the list_node to add to the list.
 *
 * The list_node does not need to be initialized; it will be overwritten.
 * Example:
 *	list_add_tail(&parent->children, &child->list);
 *	parent->num_children++;
 */
static inline void list_add_tail(struct list_head *h, struct list_node *n)
{
	n->next = &h->n;
	n->prev = h->n.prev;
	h->n.prev->next = n;
	h->n.prev = n;
	(void)debug_list(h);
}

/**
 * list_del - delete an entry from a linked list.
 * @n: the list_node to delete from the list.
 *
 * Example:
 *	list_del(&child->list);
 *	parent->num_children--;
 */
static inline void list_del(struct list_node *n)
{
	n->next->prev = n->prev;
	n->prev->next = n->next;
	(void)debug_list(n->next);
#ifdef CCAN_LIST_DEBUG
	/* Catch use-after-del. */
	n->next = n->prev = NULL;
#endif
}

/**
 * list_empty - is a list empty?
 * @h: the list_head
 *
 * If the list is empty, returns true.
 *
 * Example:
 *	assert(list_empty(&parent->children) == (parent->num_children == 0));
 */
static inline int list_empty(const struct list_head *h)
{
	(void)debug_list(h);
	return h->n.next == &h->n;
}

/**
 * list_entry - convert a list_node back into the structure containing it.
 * @n: the list_node
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * Example:
 *	struct child *c;
 *	// First list entry is children.next; convert back to child.
 *	c = list_entry(parent->children.next, struct child, list);
 */
#define list_entry(n, type, member) container_of(n, type, member)

/**
 * list_top - get the first entry in a list
 * @h: the list_head
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *first;
 *	first = list_top(&parent->children, struct child, list);
 */
#define list_top(h, type, member) \
	(list_empty(h) ? NULL : list_entry((h)->n.next, type, member))

/**
 * list_tail - get the last entry in a list
 * @h: the list_head
 * @type: the type of the entry
 * @member: the list_node member of the type
 *
 * If the list is empty, returns NULL.
 *
 * Example:
 *	struct child *last;
 *	last = list_tail(&parent->children, struct child, list);
 */
#define list_tail(h, type, member) \
	(list_empty(h) ? NULL : list_entry((h)->n.prev, type, member))

/**
 * list_for_each - iterate through a list.
 * @h: the list_head
 * @i: the structure containing the list_node
 * @member: the list_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.
 *
 * Example:
 *	struct child *c;
 *	list_for_each(&parent->children, c, list)
 *		printf("Name: %s\n", c->name);
 */
#define list_for_each(h, i, member)					\
	for (i = container_of_var(debug_list(h)->n.next, i, member);	\
	     &i->member != &(h)->n;					\
	     i = container_of_var(i->member.next, i, member))

/**
 * list_for_each_safe - iterate through a list, maybe during deletion
 * @h: the list_head
 * @i: the structure containing the list_node
 * @nxt: the structure containing the list_node
 * @member: the list_node member of the structure
 *
 * This is a convenient wrapper to iterate @i over the entire list.  It's
 * a for loop, so you can break and continue as normal.  The extra variable
 * @nxt is used to hold the next element, so you can delete @i from the list.
 *
 * Example:
 *	struct child *c, *n;
 *	list_for_each_safe(&parent->children, c, n, list) {
 *		list_del(&c->list);
 *		parent->num_children--;
 *	}
 */
#define list_for_each_safe(h, i, nxt, member)				\
	for (i = container_of_var(debug_list(h)->n.next, i, member),	\
		nxt = container_of_var(i->member.next, i, member);	\
	     &i->member != &(h)->n;					\
	     i = nxt, nxt = container_of_var(i->member.next, i, member))

#endif
