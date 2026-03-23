//Name: Muhammad Noor
//Student ID: i232520
//Assignment #: 1

#include<iostream>
#include<ctime>
#include<cmath>
using namespace std;

//assignment parameters: d1=2, d2=5, d3=2, d4=5 were provided (d4 from 0605 - 4th digit from last 4 digits of student cell)
//N =100000 + d1*10000  => 100000 + 2*10000 =120000
//epochs=100+d3*10 => 100 + 2*10 =120
//alpha = 0.001 + d2*10^-4  => 0.001 + 5*0.0001 =0.0015
//w0 = 0.1*d3  => 0.1*2 =0.2
//b0 = 0.1*d4  => 0.1*5 =0.5

const int N=120000;
const int epochs=120;
const double alpha=0.0015;
const double w0=0.2;
const double b0=0.5;
//dataset formula: y_i =(2+0.01*d1)*x_i +(1 +0.05*d2)
//used with d1=2,d2=5 to get the true underlying slope/intercept

const double true_w=2.0 +0.01 *2.0;
const double true_b=1.0 +0.05 *5.0;

//feature scaling factor (scale inputs to [0,1])
const double SCALE_FACTOR = static_cast<double>(N);

int main()
{
    //static arrays
    static double x[N];
    static double y[N];

    //dataset generation
    //x_i = i, 
    //y_i =(2+0.01*d1)*x_i +(1 + 0.05*d2)
    for (int i=0;i<N;i++)
    {
        double raw=static_cast<double>(i+1);
        double scaled=raw /SCALE_FACTOR;
        x[i]=scaled;
        y[i]=(true_w *raw +true_b)/SCALE_FACTOR;
    }

    // Initialize parameters
    double w=w0;
    double b=b0;
    clock_t elapsed_start=clock();

    //batch gradient descent (sequential)
    for (int epoch = 0;epoch <epochs;epoch++)
    {
        double sum_err_x =0.0;
        double sum_err =0.0;

        //Predicted value: y_hat_i= w*x_i + b
        //error_i =y_i - y_hat_i (note the flipped order so that the gradient keeps the -2/N factor)
        //Gradient sums: dJ/dw= -2/N *sum(error_i * x_i),
        //dJ/db= -2/N * sum(error_i)
        for (int i=0;i<N;i++)
        {
            double pred=w*x[i] + b;
            double err=y[i]-pred;
            sum_err_x +=err*x[i];
            sum_err +=err;
        }

        double dw=(-2.0 /N) *sum_err_x;
        double db=(-2.0 /N) *sum_err;

        //Update rule: w = w - alpha*dw,b = b - alpha*db
        w -=alpha *dw;
        b -=alpha *db;
    }

    clock_t elapsed_end= clock();
    double seconds=static_cast<double>(elapsed_end-elapsed_start)/CLOCKS_PER_SEC;

    cout<<"=== Sequential Results ==="<<endl;
    cout<<"Final Weight: " << w <<endl;
    cout<<"Final Bias:   " << b <<endl;
    cout<<"Time:         " << seconds << " s " <<endl;
    return 0;
}

//g++ -std=c++11 -pthread -O2 23i-2520-A-TASK1.cpp -o task1 && ./task1