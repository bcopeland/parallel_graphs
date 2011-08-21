#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <glib.h>
#include "list.h"

/*
 * Simplified K-L partitioning.  Instead of finding the (locally) best
 * swap, we find a good one based on the highest D-values, as in
 * Ravikumar, "Parallel methods for VLSI layout design."
 */

#define EXTERNAL_COST 1.0
#define INTERNAL_COST 1.0

struct cluster
{
    struct list_head vertices;
};

struct edge
{
    struct vertex *src;
    struct vertex *target;
};

struct adj
{
    int id;
    struct list_node list;
};

struct vertex
{
    bool locked;
    float cost;
    int id;
    int cluster;
    struct list_node list;
    struct list_node cluster_list;
    struct list_head next;
};

struct kl_info
{
    int nverts;
    int nclusters;
    struct cluster *clusters;
    struct vertex *verts;
    GHashTable *edge_hash;
};

struct swap_info
{
    struct vertex *a, *b;
    float gain;
};

/**
 *  Edge-search routines (hash-table based)
 */
guint edge_hash_func(gconstpointer key)
{
    const struct edge *e = key;
    int id = (e->src->id << 16) | e->target->id;
    return g_int_hash(&id);
}

gboolean edge_equal_func(gconstpointer v1, gconstpointer v2)
{
    const struct edge *e1 = v1;
    const struct edge *e2 = v2;

    return e1->src == e2->src && e1->target == e2->target;
}

static bool has_edge(struct kl_info *ki, struct vertex *v1, struct vertex *v2)
{
    struct edge e = {
        .src = v1,
        .target = v2,
    };
    return g_hash_table_lookup(ki->edge_hash, &e) != NULL;
}

static void add_edge(struct kl_info *ki, struct vertex *v1, struct vertex *v2)
{
    struct edge *e = malloc(sizeof(*e));
    e->src = v1;
    e->target = v2;
    g_hash_table_insert(ki->edge_hash, e, e);
}

/*
 *  Cluster management functions.
 */
static void cluster_add(struct kl_info *info, struct vertex *v, int cluster)
{
    v->cluster = cluster;
    list_add(&info->clusters[v->cluster].vertices, &v->cluster_list);
}

static void cluster_switch(struct kl_info *info, struct vertex *v)
{
    // printf("Moved %d to cluster %d\n", v->id, !v->cluster);
    list_del(&v->cluster_list);
    cluster_add(info, v, !v->cluster);
}

static float edge_cost(bool external)
{
    return (external) ? EXTERNAL_COST : -INTERNAL_COST;
}

static float cost_between(struct kl_info *ki, struct vertex *v1,
                          struct vertex *v2)
{

    if (has_edge(ki, v1, v2))
        return edge_cost(v1->cluster == v2->cluster);
 
    return 0;
}

/* cost is # external edges * ext_cost + # internal * int_cost */
static float cost(struct kl_info *ki, struct vertex *v)
{
    struct adj *a;

    float cost = 0;
    list_for_each(&v->next, a, list)
    {
        struct vertex *other = &ki->verts[a->id];
        cost += edge_cost(other->cluster == v->cluster);
    }
    return cost;
}

int iterate(struct kl_info *info, int max_iterations)
{
    int i, j, k;
    int nmoves = 0;
    struct swap_info *swaps;
    struct vertex *v;
    int iteration = 0;

    float gmax, gsum;
    int maxk;
    int max_moves = info->nverts / 2;

    swaps = malloc(sizeof(struct swap_info) * max_moves);

    do
    {
        fprintf(stderr, ".\n");

        /* compute d values for each v */
        for (i=0; i < info->nverts; i++)
        {
            info->verts[i].locked = false;
            info->verts[i].cost = cost(info, &info->verts[i]);
        }

        for (j=0; j < max_moves; j++)
        {
            /* find maximum cost values in both partitions and swap */
            struct swap_info *swap = &swaps[j];
            for (i=0; i < 2; i++)
            {
                struct vertex **vp = (i == 0) ? &swap->a : &swap->b;
                int max_cost = 0;

                *vp = NULL;
                struct cluster *c = &info->clusters[i];
                list_for_each(&c->vertices, v, cluster_list)
                {
                    if (!v->locked && v->cost > max_cost)
                    {   
                        *vp = v;
                        max_cost = v->cost;
                    }
                }
                if (!*vp)
                    goto done;

                (*vp)->locked = true;
            }

            /* swap the vertices */
            cluster_switch(info, swap->a);
            cluster_switch(info, swap->b);
            swap->gain = swap->a->cost + swap->b->cost -
                2 * cost_between(info, swap->a, swap->b);

            /* re-compute d values */
            for (i=0; i < info->nverts; i++)
            {
                struct vertex *v = &info->verts[i];
                if (!v->locked)
                {
                    v->cost += 2 * 
                        (cost_between(info, v, swap->a) +
                         cost_between(info, v, swap->b));
                }
            }
        }
 done:
        /* find k to maximize g sum */
        for (k=0, gsum = 0, gmax = 0; k < j; k++)
        {
            gsum += swaps[k].gain;
            if (gsum > gmax)
            {
                gmax = gsum;
                maxk = k;
            }
        }

        /* undo swaps from maxk..gsize (if < 0, undo all) */
        int undo = (gmax > 0) ? maxk + 1 : 0;

        for (; undo < j; undo++)
        {
            struct swap_info *s = &swaps[undo];

            cluster_switch(info, s->a);
            cluster_switch(info, s->b);
        }
    }
    while (gmax > 0 && (++iteration < max_iterations));
    free(swaps);
}
     
void print_clusters(struct kl_info *info)
{
    int i;
    for(i=0; i < info->nverts; i++)
    {
        printf("%d\n", info->verts[i].cluster);
    }
}

/*
 *  Load metis-format graph.
 */
void load_graph(struct kl_info *info, FILE *fp)
{
    char *line = NULL;
    char *tmp;
    char *endptr;
    int nverts, nedges;
    int count;
    int i;

    srandom(123);

    if (getline(&line, &count, fp) == -1)
        goto out;

    if (sscanf(line, "%d %d", &nverts, &nedges) != 2)
        goto out;

    free(line);
    line = NULL;

    info->nverts = nverts;
    info->verts = calloc(nverts, sizeof(struct vertex));

    for (i=0; i < info->nverts; i++)
    {
        struct vertex *v = &info->verts[i];
        v->id = i;

        cluster_add(info, v, random() & 1);
        list_head_init(&v->next);
    }

    for (i=0; i < info->nverts; i++)
    {
        if (getline(&line, &count, fp) == -1)
           break;

        struct vertex *v = &info->verts[i];

        for (tmp=line, endptr=NULL; tmp; tmp = endptr)
        {
            long neighbor = strtol(tmp, &endptr, 10);
            if (tmp == endptr)
                break;

            /* insert into adjacency list */
            struct adj *n = malloc(sizeof(*n));
            n->id = neighbor-1;
            list_add_tail(&v->next, &n->list);

            /* also store into a hash table for connectivity queries */
            add_edge(info, v, &info->verts[neighbor]);
        }
        if (line) free(line);
        line = NULL;
    }
out:
    if (line)
        free(line);
}

void print_graph(struct kl_info *ki)
{
    int i;
    for (i=0; i < ki->nverts; i++)
    {
        struct vertex *v = &ki->verts[i];
        struct adj *a;

        printf("%d -> ", v->id + 1);
        list_for_each(&v->next, a, list)
        {
            printf("%d ", a->id+1);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    struct kl_info ki;
    int iterations = 10;
    int partitions;
    FILE *fp;
    int i;

    if (argc < 3)
    {
        printf("Usage: %s <file> <nparts>\n", argv[0]);
        exit(0);
    }

    fp = fopen(argv[1], "r");
    if (!fp)
    {
        perror("open");
        exit(0);
    }

    partitions = atoi(argv[2]);

    ki.nclusters = partitions;
    ki.clusters = calloc(partitions, sizeof(struct cluster));

    ki.edge_hash = g_hash_table_new(edge_hash_func, edge_equal_func);

    for (i=0; i < partitions; i++)
        list_head_init(&ki.clusters[i].vertices);

    load_graph(&ki, fp);
    fclose(fp);

    iterate(&ki, 1);

    print_clusters(&ki);
}
