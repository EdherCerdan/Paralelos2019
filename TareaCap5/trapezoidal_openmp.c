#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

double funcion(double x)
{
    return x*x;
}

void Trapezoidal(double a, double b, int n,double *global_result){
	double h,x,resultado;
	double local_a,local_b;
	int i,local_n;
	int my_rank = omp_get_thread_num();//Devuelve el número de hilo del que se ejecuta
	int thread_count = omp_get_num_threads();//number of threads used in a parallel region

	h=(b-a)/n;
	local_n=n/thread_count;
	local_a=a+my_rank*local_n*h;
	local_b=local_a+local_n*h;
	resultado=(funcion(local_a)+funcion(local_b))/2.0;
	for(int i=1;i<=local_n-1;i++){
		x=local_a+i*h;
		resultado+=funcion(x);
	}
	resultado=resultado*h;

#	pragma omp critical //sólo un thread puede ejecutarse
	*global_result+=resultado;
}

int main(int argc, char const *argv[])
{
	double global_result=0.0;
	double a,b;
	int n,thread_count;
	thread_count = strtol(argv[1], NULL, 10); //Get number of threads from command line 
	printf("Ingrese a, b y n\n");
	scanf("%lf %lf %d",&a, &b ,&n);
#	pragma omp parallel num_threads(thread_count)
	Trapezoidal(a,b,n,&global_result);
	printf("Con n=%d trapecios\n",n);
	printf("a= %f b= %f -> resultado %.14e\n",a,b,global_result );

	return 0;
}