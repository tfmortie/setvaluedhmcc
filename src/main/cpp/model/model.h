/* 
    Author: Thomas Mortier 2019-2020

    Header shared between different models
*/

#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <vector>
#include "model/utility.h"
#include "Eigen/Dense"
#include "Eigen/SparseCore"

/* type of model */
enum class ModelType {
    SOFTMAX,
    HSOFTMAXS,
    HSOFTMAXF
};

/* type of optimizer */
enum class OptimType {
    SGD,
    ADAM
};

/* struct which contains information regarding the learning task for a model */
struct problem
{
    /* DATA */
	unsigned long n, d; /* number of instances and features (including bias) */
    std::vector<std::vector<unsigned long>> hstruct; /* structure classification problem */
    std::vector<unsigned long> y; /* vector with classes */
    std::vector<Eigen::SparseVector<double>> X; /* vector with sparse vectors */
	double bias; /* < 0 if no bias */
    /* LEARNING */
    OptimType optim; /* optimizer */
    unsigned int ne; /* number of epochs for training */
    double lr; /* learning rate for training */
    unsigned int batchsize; /* mini-batch size */
    int patience; /* patience for early stopping */
    bool fast; /* fast backprop (h-softmax) */
    /* PREDICTING */
    param utility;
};

/* superclass model */
class Model 
{
    protected:
        problem* prob;
        
    public:
        Model(problem* prob) : prob{prob} {};
        Model(const char* model_file_name, problem* prob) : prob{prob} {};
        virtual ~Model() {};

        virtual void printStruct() = 0;
        virtual void printInfo(const bool verbose = 0) = 0;
        virtual void performCrossValidation(unsigned int k) = 0;
        virtual void reset() = 0;
        virtual void fit(const std::vector<unsigned long>& ign_index = {}, const bool verbose = 1) = 0;
        virtual unsigned long predict(const Eigen::SparseVector<double>& x) = 0;
        virtual std::vector<double> predict_proba(const Eigen::SparseVector<double>& x, const std::vector<unsigned long> ind = {}) = 0;
        virtual std::vector<unsigned long> predict_ubop(const Eigen::SparseVector<double>& x) = 0;
        virtual std::vector<unsigned long> predict_rbop(const Eigen::SparseVector<double>& x) = 0;
        virtual unsigned long getNrClass() = 0;
        virtual unsigned long getNrFeatures() = 0;
        virtual void save(const char* model_file_name) = 0;
        virtual void load(const char* model_file_name) = 0; 
};

#endif