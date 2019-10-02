#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char const *argv[])
{
	double sumatoria=0,cont;
	double factor,pi_approx;
	int n, thread_count;
	thread_count= strtol(argv[1], NULL, 10); //Get number of threads from command line 

	printf("n:");
	scanf("%i",&n);

//#	pragma omp parallel for num_threads(thread_count)\
	reduction(+:sumatoria) private(factor)//private-> cada thread tiene su propia variable	
	for(int i=0;i<n;i++){
		if(i%2==0){
			factor=1.0;
		}
		else{
			factor= -1.0;
		}
		//printf("sumatoriaa1 %d", sumatoria);
		cont=factor/(2*i+1);
		sumatoria=sumatoria+cont;
		printf("factor %d \n", factor);
		printf("cont %d \n", cont);
		
	}
	pi_approx=4.0*sumatoria;

	printf("\n");
	printf("Resultado= %d ",pi_approx);
	return 0;
}
