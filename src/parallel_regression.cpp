//Name: Muhammad Noor
//Student ID: i232520
//Assignment #: 1

#include<iostream>
#include<pthread.h>
#include<ctime>
#include<cmath>
using namespace std;

//Assignment parameters: d1=2, d2=5, d3=2, d4=5 were provided (d4 from 0605 - 4th digit from last 4 digits of student cell)
//N = 100000 + d1*10000  => 100000 + 2*10000 = 120000
//epochs=100+d3*10 => 100 + 2*10 = 120
//alpha = 0.001 + d2*10^-4  => 0.001 + 5*0.0001 = 0.0015
//w0 = 0.1*d3             => 0.1*2 = 0.2
//b0 = 0.1*d4             => 0.1*5 = 0.5

const int N=120000;
const int epochs=120;
const double alpha=0.0015;
const double w0=0.2;
const double b0=0.5;
//dataset formula: y_i =(2 +0.01*d1)*x_i +(1 +0.05*d2)
//used with d1=2,d2=5 to fix the true parameters below

const double TRUE_W = 2.0 + 0.01 * 2.0;
const double TRUE_B = 1.0 + 0.05 * 5.0;

//feature scaling factor (scale inputs to [0,1])
const double SCALE_FACTOR=static_cast<double>(N);

//thread count (overriden at compile time using -DNUM_THREADS=4/8/12)
#ifndef NUM_THREADS
const int NUM_THREADS =4;
#endif

//mutex for safely updating global gradient sums
pthread_mutex_t mtx;

//static arrays
static double x[N];
static double y[N];

//shared accumulators for partial gradients
static double global_sum_err_x=0.0;
static double global_sum_err=0.0;

//computing partial gradient sums for a chunk [start,end)
struct ThreadData
{
    int start;
    int end;
    double w;
    double b;
};

//thread func to compute partial gradient sums for a chunk of data
void* compute_partial(void* arg)
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    double local_sum_err_x=0.0;
    double local_sum_err=0.0;
    double curr_w=data->w;
    double curr_b=data->b;
    int start=data->start;
    int end=data->end;

    //dJ/dw= -2/N *sum(e_i*x_i)
    //dJ/db= -2/N *sum(e_i)
    for (int i=start;i<end;i++)
    {
        double pred=curr_w *x[i] + curr_b;
        double err=y[i] - pred;
        local_sum_err_x+= err *x[i];
        local_sum_err+=err;
    }

    //combining partial gradients safely (mutex prevents race conditions)
    pthread_mutex_lock(&mtx);
    global_sum_err_x+= local_sum_err_x;
    global_sum_err+= local_sum_err;
    pthread_mutex_unlock(&mtx);
    return nullptr;
}

int main()
{
    //dataset generation
    //x_i= i,
    //y_i=(2 + 0.01*d1)*x_i + (1 + 0.05*d2)
    for (int i=0;i < N;i++)
    {
        double raw=static_cast<double>(i+1);
        double scaled = raw /SCALE_FACTOR;
        x[i]=scaled;
        y[i]=(TRUE_W *raw + TRUE_B)/SCALE_FACTOR;
    }

    //initializing parameters
    double w =w0;
    double b =b0;

    pthread_mutex_init(&mtx,nullptr);
    clock_t elapsed_start=clock();

    //batch gradient descent (parallel gradient computation)
    for (int epoch=0;epoch < epochs;epoch++)
    {
        global_sum_err_x=0.0;
        global_sum_err=0.0;

        //splitting data into T disjoint chunks and launching threads
        pthread_t threads[NUM_THREADS];
        ThreadData args[NUM_THREADS];
        int chunk = N /NUM_THREADS;
        int remainder = N% NUM_THREADS;
        int offset=0;

        for (int t=0;t <NUM_THREADS;t++)
        {
            int size=chunk +(t <remainder ?1 :0);
            args[t]={offset,offset +size,w,b};
            pthread_create(&threads[t],nullptr,compute_partial,&args[t]);
            offset+=size;
        }

        for (int t=0;t <NUM_THREADS;t++)
            pthread_join(threads[t],nullptr);
        
        //final gradients from combined sums
        double dw=(-2.0 /N) *global_sum_err_x;
        double db=(-2.0 /N) *global_sum_err;

        //update rule: w = w - alpha*dw, b = b - alpha*db
        w -=alpha* dw;
        b -=alpha* db;
    }

    clock_t elasped_end =clock();
    double seconds =static_cast<double>(elasped_end-elapsed_start)/CLOCKS_PER_SEC;

    cout<<"=== Parallel Results (Threads: " << NUM_THREADS << ") ==="<<endl;
    cout<<"Final Weight: " << w <<endl;
    cout<<"Final Bias:   " << b <<endl;
    cout<<"Time:         " << seconds << " s " <<endl;
    pthread_mutex_destroy(&mtx);
    return 0;
}

//g++ -std=c++11 -pthread -O2 -DNUM_THREADS=4 23i-2520-A-TASK2.cpp -o task2 && ./task2