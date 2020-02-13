/* 
    Author: Thomas Mortier 2019-2020

    Header utils  
*/

#ifndef UTILS_H
#define UTILS_H

#include "model/model.h"

const double EPS = 0.001; /* min. cut-off for probability (avoid log underflow) */ 

void dgemv(const double alpha, const double** W, const double* x, double* y, const unsigned long d, const unsigned long k);
void dvscalm(const double alpha, const double* x, double** D, const unsigned long d, const unsigned long k, const unsigned long i);
void dvscal(const double alpha, double* x, const unsigned long d);
void dsubmv(const double alpha, double** W, const double** D, const unsigned long d, const unsigned long k, const unsigned long i);
void softmax(double* x, const unsigned long d);
void initUW(const double min, const double max, double** W, const unsigned long d, const unsigned long k);
double* ftvToArr(const feature_node *x, const unsigned long size);

#endif