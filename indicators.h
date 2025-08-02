#ifndef indicators_h
#define indicators_h

#include "Common.h"

double calcAddEpsIndicator(ind *p_ind_a, ind *p_ind_b);
double calcIndicatorValue(ind *p_ind_a, ind *p_ind_b, int indicator, float rho, int dim, double max_bound);
int dominates(ind *p_ind_a, ind *p_ind_b);

#endif
