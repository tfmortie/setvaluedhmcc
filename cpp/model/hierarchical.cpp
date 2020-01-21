/*
Author: Thomas Mortier 2019

Implementation of hierarchical model 
*/

// TODO: finalize comments
// TODO: initializers!!!!

#include "model/hierarchical.h"
#include "utils.h"
#include <iostream>
#include <utility> 
#include <algorithm>
#include <sstream>
#include <queue>
#include <cmath>
#include <iterator>
#include <vector>

HNode::HNode(const problem &prob) 
{
    // first init W matrix
    this->W = W_hnode{new double*[static_cast<unsigned long>(prob.n)], static_cast<unsigned long>(prob.n), 0};
    // init D vector
    this->D = D_hnode{new double*[static_cast<unsigned long>(prob.n)], static_cast<unsigned long>(prob.n), 0};
    // set y attribute of this node (i.e., root)
    this->y = prob.h_struct[0];
    // now construct tree
    for (unsigned int i = 1; i < prob.h_struct.size(); ++i)
        this->addChildNode(prob.h_struct[i], prob);    
}   

HNode::HNode(std::vector<int> y, const problem &prob) : y{y}
{
    // only init D if internal node!
    if (y.size() > 1)
    {
        // first init W matrix
        this->W = W_hnode{new double*[static_cast<unsigned long>(prob.n)], static_cast<unsigned long>(prob.n), 0};
        // init D vector
        this->D = D_hnode{new double*[static_cast<unsigned long>(prob.n)], static_cast<unsigned long>(prob.n), 0};
    }
} 

HNode::~HNode()
{
    this->free();
}

unsigned int HNode::predict(const feature_node *x)
{
    // forward step
    double* o {new double[this->W.k]}; // array of exp
    // convert feature_node arr to double arr
    double* x_arr {ftvToArr(x, this->W.d)}; 
    // Wtx
    dgemv(1.0, const_cast<const double**>(this->W.value), x_arr, o, this->W.d, this->W.k);
    // get index max
    double* max_o {std::max_element(o, o+this->W.k)}; 
    unsigned int max_i {static_cast<unsigned int>(static_cast<unsigned long>(max_o-o))};
    // delete
    delete[] x_arr;
    delete[] o;
    return max_i;
}

double HNode::update(const feature_node *x, const long ind, const float lr)
{
    // forward step
    double* o {new double[this->W.k]}; // array of exp
    // convert feature_node arr to double arr
    double* x_arr {ftvToArr(x, this->W.d)}; 
    // Wtx
    dgemv(1.0, const_cast<const double**>(this->W.value), x_arr, o, this->W.d, this->W.k);
    // apply softmax
    softmax(o, this->W.k); 
    // set delta's 
    double t {0.0};
    for(unsigned long i=0; i<this->D.k; ++i)
    {
        if(static_cast<const long&>(i) == ind)
            t = 1.0;
        else
            t = 0.0;
        dvscalm((o[ind]-t), x_arr, this->D.value, this->D.d, this->D.k, i);
    }
    // backward step
    this->backward(x, lr);
    double p {o[ind]};
    // delete
    delete[] x_arr;
    delete[] o;
    return p;
}

void HNode::backward(const feature_node *x, const float lr)
{
    for (unsigned long i=0; i<this->W.k; ++i)
        dsubmv(lr, this->W.value, const_cast<const double**>(this->D.value), this->W.d, this->W.k, i);
}

void HNode::addChildNode(std::vector<int> y, const problem &prob)
{
    // todo: optimize?
    // check if leaf or internal node 
    if (this->chn.size() > 0)
    {
        // check if y is a subset of one of the children
        int ind = -1;
        for (unsigned int i = 0; i < this->chn.size(); ++i)
        {
            if (std::includes(this->chn[i]->y.begin(), this->chn[i]->y.end(), y.begin(), y.end()) == 1)
            {
                ind = static_cast<int>(i);
                break;
            }
        }
        if (ind != -1)
            // subset found, hence, recursively pass to child
            this->chn[static_cast<unsigned long>(ind)]->addChildNode(y, prob);
        else
        {
            // no children for which y is a subset, hence, put in children list
            HNode* new_node = new HNode{y, prob};
            this->chn.push_back(new_node);
            unsigned long tot_len_y_chn {0};
            for (auto c : this->chn)
                tot_len_y_chn += c->y.size();
            // check if the current node has all its children
            if (tot_len_y_chn == this->y.size())
            {
                // allocate weight and delta vectors 
                for (unsigned int i=0; i < static_cast<unsigned int>(prob.n); ++i)
                {
                    this->W.value[i] = new double[this->chn.size()];
                    this->D.value[i] = new double[this->chn.size()]{0};
                }
            }
            // set k size attribute
            this->W.k = this->chn.size();
            this->D.k = this->chn.size();
            // init W
            initUW(static_cast<double>(-1.0/this->W.d), static_cast<double>(1.0/this->W.d), this->W.value, this->W.d, this->W.k);
        }
    }
    else
    { 
        // no children yet, hence, put in children list
        HNode* new_node = new HNode{y, prob};
        this->chn.push_back(new_node);
    }
}

void HNode::free()
{
    if (this->y.size() > 1)
    { 
        for (unsigned int i = 0; i < static_cast<unsigned int>(this->W.d); ++i)
        {
            delete[] this->W.value[i];
            delete[] this->D.value[i];
        }
        delete[] this->W.value;
        delete[] this->D.value;
    }
}  

void HNode::print()
{
    std::ostringstream oss;
    if(!this->y.empty())
    {
        // Convert all but the last element to avoid a trailing ","
        std::copy(this->y.begin(), this->y.end()-1,
        std::ostream_iterator<int>(oss, ","));

        // Now add the last element with no delimiter
        oss << this->y.back();
    }
    if (this->chn.size() != 0)
    {
        std::cout << "NODE(" << oss.str() << ")\n";
        std::cout << "[\n" ;
        for (auto c : this->chn)
            c->print();
        std::cout << "]\n";
    } 
    else
        std::cout << "NODE(" << oss.str() << ")\n";
}

HierModel::HierModel(const problem &prob) : prob{prob}
{
    // construct tree 
    root = new HNode(prob);
}

HierModel::~HierModel()
{
    if (root != nullptr)
    {
        std::queue<HNode*> visit_list; 
        visit_list.push(this->root);
        while(!visit_list.empty())
        {
            HNode* visit_node = visit_list.front();
            visit_list.pop();
            if (!visit_node->chn.empty())
            {
                for(auto* c : visit_node->chn)
                    visit_list.push(c);
            }
            // free node
            delete visit_node;
        }
    }
}

void HierModel::printStruct()
{
    std::cout << "[info] Structure:\n";
    for (auto &v : this->prob.h_struct)
    {
        std::cout << "[";
        for (auto &el: v)
        {
            std::cout << el << ",";
        }
        std::cout << "]\n";
    }
}

void HierModel::print()
{
    if (root != nullptr)
        this->root->print();
}

void HierModel::printInfo()
{
    std::cout << "---------------------------------------------------\n";
    std::cout << "[info] Hierarchical model: \n";
    std::cout << "---------------------------------------------------\n";
    std::cout << "  * Number of features              = " << this->prob.n << '\n';
    std::cout << "  * Number of samples               = " << this->prob.l << '\n';
    std::cout << "  * Model =\n";
    this->print();
    std::cout << "---------------------------------------------------\n\n";
}

void HierModel::performCrossValidation()
{
    //TODO: implement!
}

void HierModel::fit(const unsigned int ne, const float lr)
{
    if (root != nullptr)
    {
        unsigned int cntr {0};
        while(cntr<ne)
        {
            double e_loss {0.0};
            // run over each instance 
            //for (unsigned int n {0};n<10; ++n) /* debug purposes */
            for (unsigned int n = 0; n<static_cast<unsigned int>(this->prob.l); ++n)
            {
                double i_loss {0.0};
                feature_node* x {this->prob.x[n]};
                std::vector<int> y {(int) this->prob.y[n]}; // our class 
                //std::cout << "Y = " << y[0] << '\n';
                HNode* visit_node = this->root;
                while(!visit_node->chn.empty())
                {
                    int ind = -1;
                    for (unsigned int i = 0; i < visit_node->chn.size(); ++i)
                    { 
                        if (std::includes(visit_node->chn[i]->y.begin(), visit_node->chn[i]->y.end(), y.begin(), y.end()) == 1)
                        {
                            ind = static_cast<int>(i);
                            break;
                        }  
                    }
                    if (ind != -1)
                    {
                        double i_p {visit_node->update(x, static_cast<long>(ind), lr)};
                        //std::cout << "i_p: " << i_p << '\n';
                        i_loss += std::log2((i_p<=EPS ? EPS : i_p));
                        visit_node = visit_node->chn[static_cast<unsigned long>(ind)];
                    }
                    else
                    {
                        std::cerr << "[error] label " << y[0] << " not found in hierarchy!\n";
                        exit(1);
                    }
                }
                //std::cout << "i_loss: " << -i_loss << '\n';
                e_loss += -i_loss;
            }
            std::cout << "Epoch " << cntr << ": loss " << (e_loss/static_cast<double>(this->prob.l)) << '\n';
            ++cntr;
        }
    }
    else
    {
        std::cerr << "[warning] Model has not been fitted yet\n";
        exit(1);
    }
}

// predict class with highest probability 
double HierModel::predict(const feature_node *x)
{
    if (root == nullptr)
    {
        std::cerr << "[warning] Model has not been fitted yet";
        return -1.0;
    }
    else
    {
        HNode* visit_node = this->root;
        while(!visit_node->chn.empty())
           visit_node = visit_node->chn[visit_node->predict(x)];
        
        return static_cast<double>(visit_node->y[0]);
    }
}

void HierModel::predict_proba(const feature_node* x, double* prob_estimates)
{
    if (root == nullptr)
    {
        std::cerr << "[warning] Model has not been fitted yet!";
    }
    else
    {
        // fill in probability estimates vector 
        // TODO: implement!
    }
}
    
// check problem and parameter before training
void HierModel::checkParam()
{
    // TODO: implement!
}

// get number of classes of fitted model
int HierModel::getNrClass()
{
    if (root == nullptr)
    {
        std::cerr << "[warning] Model has not been fitted yet!\n";
        return 0;
    }
    else
        return this->root->y.size();
}

// save model
void HierModel::save(const char* model_file_name)
{
    // TODO: implement!
}