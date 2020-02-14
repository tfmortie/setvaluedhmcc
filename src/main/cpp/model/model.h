/* 
    Author: Thomas Mortier 2019-2020

    Header shared between different models
*/

#ifndef MODEL_U
#define MODEL_U

#include <iostream>
#include <vector>

/* type of model */
enum class ModelType {
    SOFTMAX,
    HSOFTMAX
};

/* struct which allows for sparse feature representations */
struct feature_node
{
	long index;
	double value;
};

/* struct which contains learning problem specific information */
struct problem
{
    /* DATA */
	unsigned long n, d; /* number of instances and features (including bias) */
    std::vector<std::vector<unsigned long>> hstruct; /* structure classification problem */
	unsigned long *y; /* vector with classes */
	feature_node **X; /* array with instances */
	double bias; /* < 0 if no bias */
    /* LEARNING */
    unsigned int ne; /* number of epochs for training (SGD) */
    double lr; /* learning rate for training (SGD) */
};

/* matrix container for weight and delta matrices */
struct Matrix
{
    double** value; /* should be D x K */
    unsigned long d; /* D */
    unsigned long k; /* K */
};

/* superclass model */
class Model 
{
    protected:
        const problem* prob;
        
    public:
        Model(const problem* prob) : prob{prob} {};
        Model(const char* model_file_name) : prob{nullptr} {};
        virtual ~Model() {};

        virtual void printStruct() = 0;
        virtual void printInfo(const bool verbose = 0) = 0;
        virtual void performCrossValidation(unsigned int k) = 0;
        virtual void reset() = 0;
        virtual void fit(const std::vector<unsigned long>& ign_index = {}, const bool verbose = 1) = 0;
        virtual unsigned long predict(const feature_node* x) = 0;
        virtual std::vector<unsigned long> predict_proba(const feature_node* x, const std::vector<unsigned long> ind = {}) = 0;
        virtual unsigned long getNrClass() = 0;
        virtual unsigned long getNrFeatures() = 0;
        virtual void save(const char* model_file_name) = 0;
        virtual void load(const char* model_file_name) = 0; /* TODO: include error handling! */
};

#endif