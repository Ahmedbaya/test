#include "mokp_core.h"

// Global variables definition (from main.c)
int dimension = 2;
int NBITEMS = 250;
double capacities[MAX_DIMENSION];
int weights[MAX_DIMENSION][MAX_NBITEMS];
int profits[MAX_DIMENSION][MAX_NBITEMS];
int nf, ni, cardP;
int nombr;
int paretoIni = 28000;
pop *archive = NULL;
pop *P = NULL;
pop *solutions = NULL;
double perturbation_rate = 0.05;
int *triI;
double U2[MAX_NBITEMS];
FILE *Wfile;
double ReferencePoint[MAX_DIMENSION];
double vector_weight[MAX_DIMENSION];
double max_bound;
double OBJ_Weights[MAX_DIMENSION][10000];
int nombreLIGNE = 0;
int nextLn = 0;
int inv = 0;
float smallValue = 0.0000001f;
float max_value = 1000000.0f;
double kappa = 0.05;
int alpha = 10;
range *bounds = NULL;
int inter;
int iseed;

// Load problem from file (from main.c)
void loadMOKP(char *s) {
    FILE* source;
    int i, f;
    char cl[20];
    
    source = fopen(s, "r");
    if (!source) {
        printf("Error: Cannot open file %s\n", s);
        return;
    }
    
    fscanf(source, " %d %d  \n", &nf, &ni);
    printf(" %d %d  \n ", nf, ni);
    
    for (f = 0; f < nf; f++) {
        fscanf(source, "%lf  \n ", &capacities[f]);
        
        for (i = 0; i < ni; i++) {
            fscanf(source, " %s \n", cl);
            fscanf(source, " %d  \n", &weights[f][i]);
            fscanf(source, "  %d  \n ", &profits[f][i]);
        }
    }
    fclose(source);
}

// Memory allocation wrapper (from main.c)
void* chk_malloc(size_t size) {
    void *return_value = malloc(size);
    if (return_value == NULL)
        printf("Selector: Out of memory.");
    return (return_value);
}

// Create population (from main.c)
pop* create_pop(int maxsize, int nf) {
    int i;
    pop *pp;
    
    assert(nf >= 0);
    assert(maxsize >= 0);
    
    pp = (pop*) chk_malloc(sizeof(pop));
    pp->size = 0;
    pp->maxsize = maxsize;
    pp->ind_array = (ind**) chk_malloc(maxsize * sizeof(ind*));
    
    for (i = 0; i < maxsize; i++)
        pp->ind_array[i] = NULL;
    
    return (pp);
}

// Create individual (from main.c)
ind* create_ind(int nf) {
    ind *p_ind;
    
    assert(nf >= 0);
    assert(ni >= 0);
    
    p_ind = (ind*) chk_malloc(sizeof(ind));
    p_ind->nombr_nonpris = 0;
    p_ind->nombr = 0;
    p_ind->rank = 0;
    p_ind->fitness = -1.0;
    p_ind->fitnessbest = -1.0;
    p_ind->explored = 0;
    p_ind->f = (double*) chk_malloc(nf * sizeof(double));
    p_ind->capa = (double*) chk_malloc(nf * sizeof(double));
    p_ind->v = (double*) chk_malloc(nf * sizeof(double));
    p_ind->d = (int*) chk_malloc(ni * sizeof(int));
    p_ind->Items = (int*) chk_malloc(ni * sizeof(int));
    
    return (p_ind);
}

// Copy individual (from main.c)
ind* ind_copy(ind *i) {
    ind *p_ind = NULL;
    int k;
    
    p_ind = create_ind(nf);
    
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

// Free individual memory (from main.c)
void free_ind(ind *p_ind) {
    assert(p_ind != NULL);
    free(p_ind->d);
    free(p_ind->f);
    free(p_ind->capa);
    free(p_ind->v);
    free(p_ind->Items);
    free(p_ind);
}

// Free population memory (from main.c)
void free_pop(pop *pp) {
    if (pp != NULL) {
        free(pp->ind_array);
        free(pp);
    }
}

// Complete free population (from main.c)
void complete_free_pop(pop *pp) {
    int i = 0;
    if (pp != NULL) {
        if (pp->ind_array != NULL) {
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

// Dominance check (from main.c)
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

// Non-dominated check (from main.c)
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

// Random number generation functions (from main.c)
void seed(unsigned int seed) { 
    iseed = seed; 
    srand(seed);
}

double drand(double range) {
    double j;
    j = (range * (double) rand() / (RAND_MAX + 1.0));
    return (j);
}

int irand(int range) {
    int j;
    j = (int) ((double)range * (double) rand() / (RAND_MAX + 1.0));
    return (j);
}

int rand_a_b(int a, int b) {
    return rand() % (b - a) + a;
}

// Population initialization (from main.c)
void init_pop(pop *SP, int size) {
    int i;
    SP->size = size;
    random_init_pop(SP, size);
    for (i = 0; i < SP->size; i++)
        SP->ind_array[i]->explored = 0;
}

void random_init_pop(pop *SP, int size) {
    int i;
    SP->size = size;
    for (i = 0; i < SP->size; i++) {
        SP->ind_array[i] = create_ind(nf);
        random_init_ind(SP->ind_array[i]);
        evaluate(SP->ind_array[i]);
    }
}

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

// Evaluation function (from main.c)
void evaluate(ind *x) {
    int j, i, l, k, faisable;
    
    x->nombr = 0;
    x->nombr_nonpris = 0;
    
    for (j = 0; j < nf; j++) {
        x->capa[j] = 0;
        x->f[j] = 0;
    }
    
    for (j = 0; j < ni; j++) {
        x->Items[j] = 0;
    }
    
    for (j = 0; j < ni; j++) {
        l = 0;
        faisable = 1;
        do {
            if (x->capa[l] + weights[l][x->d[j]] > capacities[l])
                faisable = 0;
            l++;
        } while ((l < nf) && (faisable == 1));
        
        if (faisable == 1) {
            for (k = 0; k < nf; k++) {
                x->capa[k] = x->capa[k] + weights[k][x->d[j]];
                x->f[k] = x->f[k] + profits[k][x->d[j]];
            }
            x->Items[x->d[j]] = 1;
            x->nombr++;
        } else if (faisable == 0) {
            x->nombr_nonpris++;
        }
    }
}

// Weight management functions (from main.c)
void read_weights_file(char *filename) {
    FILE *file;
    int i, j;
    
    file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open weights file %s\n", filename);
        return;
    }
    
    nombreLIGNE = 0;
    while (!feof(file)) {
        for (i = 0; i < dimension; i++) {
            if (fscanf(file, "%lf", &OBJ_Weights[i][nombreLIGNE]) != 1) {
                break;
            }
        }
        if (i == dimension) {
            nombreLIGNE++;
        }
    }
    
    fclose(file);
}

void dynamic_weight_allpop() {
    int i;
    
    for (i = 0; i < dimension; i++) {
        vector_weight[i] = OBJ_Weights[i][nextLn];
    }
    
    if (nextLn == nombreLIGNE) {
        nextLn = 0;
    } else {
        nextLn++;
    }
}

void choose_weight(void) {
    dynamic_weight_allpop();
}

// Fitness calculation functions (from main.c)
void calcul_weight(pop *SP, int size) {
    int i, j;
    
    for (i = 0; i < SP->size; i++) {
        for (j = 0; j < nf; j++) {
            SP->ind_array[i]->v[j] = SP->ind_array[i]->f[j] * vector_weight[j];
        }
    }
}

void calcMaxbound(pop* SP, int size) {
    SP->size = size;
    int i, j;
    
    max_bound = SP->ind_array[0]->v[0];
    for (i = 0; i < SP->size; i++) {
        for (j = 0; j < nf; j++) {
            if (max_bound < SP->ind_array[i]->v[j]) {
                max_bound = SP->ind_array[i]->v[j];
            }
        }
    }
}

void init_fitness(ind *x) {
    x->fitness = 0.0;
}

int delete_fitness(ind *x, double I) {
    x->fitness += exp(-I / kappa);
    return 0;
}

int update_fitness(ind *x, double I) {
    x->fitness += exp(-I / kappa);
    return 0;
}

void compute_ind_fitness(ind *x, pop *SP) {
    int i;
    double indicator_value;
    
    init_fitness(x);
    for (i = 0; i < SP->size; i++) {
        indicator_value = calcIndicatorValue(SP->ind_array[i], x, 0, 0.05f, nf, max_bound);
        update_fitness(x, indicator_value);
    }
}

void compute_all_fitness(pop *SP) {
    int i;
    for (i = 0; i < SP->size; i++) {
        compute_ind_fitness(SP->ind_array[i], SP);
    }
}

int compute_fitness_and_select(pop *SP, ind *x, int size) {
    int i, worst = 0;
    double worst_fit, fit_tmp;
    
    compute_ind_fitness(x, SP);
    worst_fit = SP->ind_array[0]->fitness;
    
    for (i = 1; i < SP->size; i++) {
        if (SP->ind_array[i]->fitness < worst_fit) {
            worst_fit = SP->ind_array[i]->fitness;
            worst = i;
        }
    }
    
    fit_tmp = x->fitness;
    if (fit_tmp > worst_fit) {
        for (i = 0; i < SP->size; i++) {
            if (delete_fitness(SP->ind_array[i], calcIndicatorValue(SP->ind_array[worst], SP->ind_array[i], 0, 0.05f, nf, max_bound)))
                compute_ind_fitness(SP->ind_array[i], SP);
            update_fitness(SP->ind_array[i], calcIndicatorValue(x, SP->ind_array[i], 0, 0.05f, nf, max_bound));
        }
        if (delete_fitness(x, calcIndicatorValue(SP->ind_array[worst], x, 0, 0.05f, nf, max_bound)))
            compute_ind_fitness(x, SP);
        free_ind(SP->ind_array[worst]);
        SP->ind_array[worst] = ind_copy(x);
        if (fit_tmp - worst_fit > smallValue)
            return worst;
        else return -1;
    }
    return -1;
}

// Population management functions (from main.c)
int extractPtoArchive(pop *P, pop *archive) {
    int i, j, dom;
    int t = archive->size + P->size;
    pop *archiveAndP;
    int convergence_rate = 0;
    
    archiveAndP = create_pop(t, nf);
    for (i = 0; i < archive->size; i++) {
        archiveAndP->ind_array[i] = archive->ind_array[i];
    }
    
    for (i = 0; i < P->size; i++) {
        archiveAndP->ind_array[i + archive->size] = ind_copy(P->ind_array[i]);
    }
    archiveAndP->size = t;
    archive->size = 0;
    
    for (i = 0; i < t; i++) {
        for (j = 0; j < t; j++) {
            if (i != j) {
                dom = non_dominated(archiveAndP->ind_array[i], archiveAndP->ind_array[j]);
                if (dom == -1 || (dom == 0 && i > j)) j = t + 1;
            }
        }
        if (j == t) {
            archive->ind_array[archive->size++] = ind_copy(archiveAndP->ind_array[i]);
            if (i >= t - P->size) convergence_rate++;
        }
    }
    
    complete_free_pop(archiveAndP);
    return convergence_rate;
}

void P_init_pop(pop *SP, pop *Sarchive, int alpha) {
    int i, x, tmp;
    int t = (alpha > Sarchive->size) ? alpha : Sarchive->size;
    int *shuffle = (int*)malloc(t * sizeof(int));
    
    SP->size = alpha;
    for (i = 0; i < t; i++) shuffle[i] = i;
    for (i = 0; i < t; i++) {
        x = irand(alpha);
        tmp = shuffle[i];
        shuffle[i] = shuffle[x];
        shuffle[x] = tmp;
    }
    
    if (Sarchive->size > alpha) {
        for (i = 0; i < alpha; i++) {
            SP->ind_array[i] = ind_copy(Sarchive->ind_array[shuffle[i]]);
        }
    } else {
        for (i = 0; i < alpha; i++) {
            if (shuffle[i] < Sarchive->size) {
                SP->ind_array[i] = ind_copy(Sarchive->ind_array[shuffle[i]]);
            } else {
                SP->ind_array[i] = create_ind(nf);
                random_init_ind(SP->ind_array[i]);
                evaluate(SP->ind_array[i]);
            }
        }
    }
    
    free(shuffle);
}

// Local search function (simplified from main.c)
void Indicator_local_search1(pop *SP, pop *Sarchive, int size) {
    ind *y;
    ind *x;
    int i, j, r, t, k, l, v, sol, mino, mp, maxp, consistant, pos, stop, convergence, ii;
    int tmp_pris, tmp_nonpris, remplace[L], taille, feasible;
    SP->size = size;
    
    extractPtoArchive(SP, Sarchive);
    
    do {
        convergence = 0;
        
        for (i = 0; i < SP->size; i++) {
            if (!SP->ind_array[i]->explored) {
                x = ind_copy(SP->ind_array[i]);
                
                for (j = 0; j < x->nombr; j++) {
                    for (l = 0; l < L; l++) {
                        remplace[l] = 0;
                    }
                    
                    do {
                        mino = irand(NBITEMS);
                    } while (x->Items[mino] == 0);
                    
                    x->Items[mino] = 0;
                    x->nombr--;
                    x->nombr_nonpris++;
                    
                    for (r = 0; r < nf; r++) {
                        x->capa[r] = x->capa[r] - weights[r][mino];
                        x->f[r] = x->f[r] - profits[r][mino];
                    }
                    
                    int IM = 0;
                    stop = 1;
                    taille = 0;
                    do {
                        do {
                            maxp = irand(NBITEMS);
                        } while (x->Items[maxp] == 1);
                        
                        if (maxp != mino) {
                            consistant = 1;
                            r = 0;
                            do {
                                if (x->capa[r] + weights[r][maxp] > capacities[r])
                                    consistant = 0;
                                r++;
                            } while ((r < nf) && (consistant == 1));
                            
                            if (consistant == 1) {
                                feasible = 1;
                                r = 0;
                                do {
                                    if (maxp == remplace[r])
                                        feasible = 0;
                                    r++;
                                } while ((r < taille) && (feasible));
                                
                                if (feasible == 1) {
                                    remplace[taille] = maxp;
                                    taille++;
                                    
                                    x->Items[maxp] = 1;
                                    x->nombr_nonpris--;
                                    x->nombr++;
                                    
                                    for (r = 0; r < nf; r++) {
                                        x->capa[r] = x->capa[r] + weights[r][maxp];
                                        x->f[r] = x->f[r] + profits[r][maxp];
                                    }
                                }
                            }
                        }
                        
                        IM++;
                    } while (IM < L);
                    
                    int tv;
                    for (tv = 0; tv < nf; tv++) {
                        x->v[tv] = x->f[tv] * vector_weight[tv];
                    }
                    
                    calcMaxbound(SP, SP->size);
                    sol = compute_fitness_and_select(SP, x, SP->size);
                    
                    if (sol != -1) {
                        j = x->nombr + 1;
                        if (sol > i) {
                            y = SP->ind_array[i + 1];
                            SP->ind_array[i + 1] = SP->ind_array[sol];
                            SP->ind_array[sol] = y;
                            i++;
                        }
                    } else if (sol == -1) {
                        x->Items[mino] = 1;
                        x->nombr_nonpris--;
                        x->nombr++;
                        
                        for (r = 0; r < nf; r++) {
                            x->capa[r] = x->capa[r] + weights[r][mino];
                            x->f[r] = x->f[r] + profits[r][mino];
                        }
                        
                        if (taille >= 1) {
                            for (r = 0; r < taille; r++) {
                                x->Items[remplace[r]] = 0;
                                x->nombr--;
                                x->nombr_nonpris++;
                                
                                for (t = 0; t < nf; t++) {
                                    x->capa[t] = x->capa[t] - weights[t][remplace[r]];
                                    x->f[t] = x->f[t] - profits[t][remplace[r]];
                                    x->v[t] = x->f[t] * vector_weight[t];
                                }
                            }
                        }
                    }
                }
                
                tmp_pris = x->nombr;
                tmp_nonpris = x->nombr_nonpris;
                free_ind(x);
                
                if (j == tmp_pris) SP->ind_array[i]->explored = 1;
            }
            j = convergence;
            convergence = extractPtoArchive(SP, Sarchive);
        }
    } while (convergence);
}

int max(int a, int b) {
    if (a > b) return a; 
    else return b;
}