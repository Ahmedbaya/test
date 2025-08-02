#include "knapsack_core.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Global variables needed by the algorithm */
double *capacities = NULL;
int **weights = NULL;
int **profits = NULL;
int nf = 0, ni = 0;

double perturbation_rate = 0.05;
double kappa = 0.05;
double vector_weight[4] = {0.5, 0.5, 0.0, 0.0}; /* Default weights for 2 objectives */

/* Memory management functions */
void* chk_malloc(size_t size) {
    void *return_value = malloc(size);
    if(return_value == NULL) {
        printf("Knapsack: Out of memory.\n");
    }
    return (return_value);
}

pop* create_pop(int maxsize, int nf_param) {
    int i;
    pop *pp;
    (void)nf_param; /* Suppress unused parameter warning */

    pp = (pop*) chk_malloc(sizeof(pop));
    if (!pp) return NULL;
    
    pp->size = 0;
    pp->maxsize = maxsize;
    pp->ind_array = (ind**) chk_malloc(maxsize * sizeof(ind*));

    if (!pp->ind_array) {
        free(pp);
        return NULL;
    }

    for (i = 0; i < maxsize; i++)
        pp->ind_array[i] = NULL;

    return (pp);
}

ind* create_ind(int nf_param) {
    ind *p_ind;

    p_ind = (ind*) chk_malloc(sizeof(ind));
    if (!p_ind) return NULL;
    
    p_ind->nombr_nonpris = 0;
    p_ind->nombr = 0;
    p_ind->rank = 0;
    p_ind->fitness = -1.0;
    p_ind->fitnessbest = -1.0;
    p_ind->explored = 0;
    
    p_ind->f = (double*) chk_malloc(nf_param * sizeof(double));
    p_ind->capa = (double*) chk_malloc(nf_param * sizeof(double));
    p_ind->v = (double*) chk_malloc(nf_param * sizeof(double));
    p_ind->d = (int*) chk_malloc(ni * sizeof(int));
    p_ind->Items = (int*) chk_malloc(ni * sizeof(int));

    if (!p_ind->f || !p_ind->capa || !p_ind->v || !p_ind->d || !p_ind->Items) {
        if (p_ind->f) free(p_ind->f);
        if (p_ind->capa) free(p_ind->capa);
        if (p_ind->v) free(p_ind->v);
        if (p_ind->d) free(p_ind->d);
        if (p_ind->Items) free(p_ind->Items);
        free(p_ind);
        return NULL;
    }

    return (p_ind);
}

void free_ind(ind *p_ind) {
    if (p_ind != NULL) {
        if (p_ind->d) free(p_ind->d);
        if (p_ind->f) free(p_ind->f);
        if (p_ind->capa) free(p_ind->capa);
        if (p_ind->v) free(p_ind->v);
        if (p_ind->Items) free(p_ind->Items);
        free(p_ind);
    }
}

void complete_free_pop(pop *pp) {
    int i = 0;
    if (pp != NULL) {
        if(pp->ind_array != NULL) {
            for (i = 0; i < pp->size; i++) {
                if (pp->ind_array[i] != NULL) {
                    free_ind(pp->ind_array[i]);
                    pp->ind_array[i] = NULL;
                }
            }
            free(pp->ind_array);
        }
        free(pp);
    }
}

ind* ind_copy(ind *i) {
    ind *p_ind = NULL;
    int k;

    p_ind = create_ind(nf);
    if (!p_ind) return NULL;

    p_ind->nombr_nonpris = i->nombr_nonpris;
    p_ind->nombr = i->nombr;
    p_ind->rank = i->rank;
    p_ind->fitnessbest = i->fitnessbest;
    p_ind->fitness = i->fitness;
    p_ind->explored = i->explored;

    for (k = 0; k < nf; k++) {
        p_ind->f[k] = i->f[k];
        p_ind->v[k] = i->v[k];
        p_ind->capa[k] = i->capa[k];
    }

    for (k = 0; k < ni; k++) {
        p_ind->d[k] = i->d[k];
        p_ind->Items[k] = i->Items[k];
    }

    return (p_ind);
}

/* Random number generators */
int irand(int range) {
    int j;
    j = (int) ((double)range * (double) rand() / (RAND_MAX + 1.0));
    return (j);
}

/* Individual evaluation */
void evaluate(ind *x) {
    int j, l, k, faisable;

    x->nombr = 0; 
    x->nombr_nonpris = 0;
    
    for(j = 0; j < nf; j++) {
        x->capa[j] = 0; 
        x->f[j] = 0;
    }

    for (j = 0; j < ni; j++) {
        l = 0; 
        faisable = 1;
        
        do {
            if (x->capa[l] + weights[l][x->d[j]] > capacities[l])
                faisable = 0;
            l++;
        } while((l < nf) && (faisable == 1));

        if (faisable == 1) {
            for (k = 0; k < nf; k++) {
                x->capa[k] = x->capa[k] + weights[k][x->d[j]];
                x->f[k] = x->f[k] + profits[k][x->d[j]];
            }
            x->Items[x->d[j]] = 1;
            x->nombr++;
        } else {
            x->Items[x->d[j]] = 0;
            x->nombr_nonpris++;
        }
    }
}

/* Random initialization */
void random_init_ind(ind *x) {
    int j, r, tmp;

    for (j = 0; j < ni; j++)
        x->d[j] = j;
        
    for (j = 0; j < ni; j++) {
        r = irand(ni);
        tmp = x->d[r];
        x->d[r] = x->d[j];
        x->d[j] = tmp;
    }
}

void random_init_pop(pop *SP, int size) {
    int i;
    SP->size = size;
    
    for (i = 0; i < SP->size; i++) {
        SP->ind_array[i] = create_ind(nf);
        if (!SP->ind_array[i]) {
            SP->size = i; /* Adjust size to valid individuals */
            return;
        }
        random_init_ind(SP->ind_array[i]);
        evaluate(SP->ind_array[i]);
    }
}

/* Dominance checking */
int dominates(ind *p_ind_a, ind *p_ind_b) {
    int i;
    int a_is_worse = 0;
    int equal = 1;

    for (i = 0; i < nf && !a_is_worse; i++) {
        a_is_worse = p_ind_a->f[i] > p_ind_b->f[i];
        equal = (p_ind_a->f[i] == p_ind_b->f[i]) && equal;
    }

    return (!equal && !a_is_worse);
}

int non_dominated(ind *p_ind_a, ind *p_ind_b) {
    int i;
    int a_is_good = -1;
    int equal = 1;

    for (i = 0; i < nf; i++) {
        if (p_ind_a->f[i] > p_ind_b->f[i]) a_is_good = 1;
        equal = (p_ind_a->f[i] == p_ind_b->f[i]) && equal;
    }
    if (equal) return 0;
    return a_is_good;
}

/* Extract Pareto front */
int extractPtoArchive(pop *P, pop *archive) {
    int i, j, dom;
    int t = archive->size + P->size;
    pop *archiveAndP;
    int convergence_rate = 0;

    archiveAndP = create_pop(t, nf);
    if (!archiveAndP) return 0;
    
    /* Combine archive and population */
    for (i = 0; i < archive->size; i++) {
        archiveAndP->ind_array[i] = archive->ind_array[i];
    }
    
    for (i = 0; i < P->size; i++) {
        archiveAndP->ind_array[i + archive->size] = ind_copy(P->ind_array[i]);
        if (!archiveAndP->ind_array[i + archive->size]) {
            /* Handle allocation failure */
            for (int k = archive->size; k < i + archive->size; k++) {
                if (archiveAndP->ind_array[k]) free_ind(archiveAndP->ind_array[k]);
            }
            complete_free_pop(archiveAndP);
            return 0;
        }
    }
    archiveAndP->size = t;
    archive->size = 0;

    /* Find non-dominated solutions */
    for (i = 0; i < t; i++) {
        for (j = 0; j < t; j++) {
            if (i != j) {
                dom = non_dominated(archiveAndP->ind_array[i], archiveAndP->ind_array[j]);
                if (dom == -1 || (dom == 0 && i > j)) {
                    j = t + 1;
                    break;
                }
            }
        }
        if(j == t) {
            if (archive->size < archive->maxsize) {
                archive->ind_array[archive->size++] = ind_copy(archiveAndP->ind_array[i]);
                if (i >= t - P->size) convergence_rate++;
            }
        }
    }

    /* Clean up temporary population */
    for (i = archive->size; i < t; i++) {
        if (archiveAndP->ind_array[i]) free_ind(archiveAndP->ind_array[i]);
    }
    complete_free_pop(archiveAndP);

    return convergence_rate;
}

/* Simplified local search */
void simple_local_search(pop *SP, pop *archive, int max_iterations) {
    int iter, i, j;
    
    for (iter = 0; iter < max_iterations; iter++) {
        /* Simple perturbation and evaluation */
        for (i = 0; i < SP->size; i++) {
            ind *x = SP->ind_array[i];
            
            /* Simple random perturbation - flip a few items */
            int num_changes = (int)(perturbation_rate * x->nombr) + 1;
            
            for (j = 0; j < num_changes; j++) {
                int item = irand(ni);
                x->Items[item] = 1 - x->Items[item]; /* Flip item */
            }
            
            /* Re-evaluate */
            evaluate(x);
        }
        
        /* Update archive */
        extractPtoArchive(SP, archive);
        
        /* Reinitialize some individuals if no improvement */
        if (iter % 10 == 9) {
            for (i = SP->size / 2; i < SP->size; i++) {
                free_ind(SP->ind_array[i]);
                SP->ind_array[i] = create_ind(nf);
                if (SP->ind_array[i]) {
                    random_init_ind(SP->ind_array[i]);
                    evaluate(SP->ind_array[i]);
                }
            }
        }
    }
}