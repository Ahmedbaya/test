#ifndef KNAPSACK_CORE_H
#define KNAPSACK_CORE_H

#include <stddef.h>
#include "Common.h"

/* Population structure */
typedef struct pop_st {
    int size;
    int maxsize;
    ind **ind_array;
} pop;

/* External variables */
extern double *capacities;
extern int **weights;
extern int **profits;
extern int nf, ni;
extern double perturbation_rate;
extern double kappa;
extern double vector_weight[4];

/* Core functions */
void* chk_malloc(size_t size);
pop* create_pop(int maxsize, int nf_param);
ind* create_ind(int nf_param);
void free_ind(ind *p_ind);
void complete_free_pop(pop *pp);
ind* ind_copy(ind *i);

int irand(int range);
void evaluate(ind *x);
void random_init_ind(ind *x);
void random_init_pop(pop *SP, int size);

int dominates(ind *p_ind_a, ind *p_ind_b);
int non_dominated(ind *p_ind_a, ind *p_ind_b);
int extractPtoArchive(pop *P, pop *archive);
void simple_local_search(pop *SP, pop *archive, int max_iterations);

#endif /* KNAPSACK_CORE_H */