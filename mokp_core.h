#ifndef MOKP_CORE_H
#define MOKP_CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

// Include original headers
#include "Common.h"
#include "IBMOLS.h"

// Remove Windows-specific includes for cross-platform compatibility
// #include <conio.h>  // Removed for Linux compatibility

#define MAX_DIMENSION 10
#define MAX_NBITEMS 1000
#define FREQUANCY 200
#define L 5
#define LARGE 10e50
#define FALSE 0
#define TRUE 1

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// Global variables (managed internally)
extern int dimension;
extern int NBITEMS;
extern double capacities[MAX_DIMENSION];
extern int weights[MAX_DIMENSION][MAX_NBITEMS];
extern int profits[MAX_DIMENSION][MAX_NBITEMS];
extern int nf, ni, cardP;
extern int nombr;
extern int paretoIni;
extern pop *archive;
extern pop *P;
extern pop *solutions;
extern double perturbation_rate;
extern double ReferencePoint[MAX_DIMENSION];
extern double vector_weight[MAX_DIMENSION];
extern double max_bound;
extern double OBJ_Weights[MAX_DIMENSION][10000];
extern int nombreLIGNE;
extern int nextLn;
extern int inv;
extern float smallValue;
extern float max_value;
extern double kappa;
extern int alpha;
extern range *bounds;

// Core algorithm functions
void* chk_malloc(size_t size);
pop* create_pop(int maxsize, int nf);
ind* create_ind(int nf);
ind* ind_copy(ind *i);
void free_ind(ind *p_ind);
void free_pop(pop *pp);
void complete_free_pop(pop *pp);
int dominates(ind *p_ind_a, ind *p_ind_b);
void init_pop(pop *SP, int size);
void random_init_pop(pop *SP, int size);
void random_init_ind(ind *x);
void evaluate(ind *x);
void loadMOKP(char *s);
void choose_weight(void);
void calcul_weight(pop *SP, int size);
void calcMaxbound(pop* SP, int size);
void compute_all_fitness(pop *SP);
void compute_ind_fitness(ind *x, pop *SP);
int compute_fitness_and_select(pop *SP, ind *x, int size);
void Indicator_local_search1(pop *SP, pop *Sarchive, int size);
int extractPtoArchive(pop *P, pop *archive);
void P_init_pop(pop *SP, pop *Sarchive, int alpha);
void read_weights_file(char *filename);
int max(int a, int b);
double drand(double range);
int irand(int range);
void seed(unsigned int seed);

#endif // MOKP_CORE_H