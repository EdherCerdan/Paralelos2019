#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

void read_vector(double *vector,long columna){
	srand(time(NULL));
	for(int i=0;i<columna;i++){
		vector[i]=rand()%10;
	}
}

void read_matriz(double *matriz,long fila, long columna){
	srand(time(NULL));
	for(int i=0;i<fila;i++){
   		for(int j=0;j<columna;j++){
			matriz[(i*columna)+j]=rand()%10;
		}
   	}
}
	
int main(int argc, char const *argv[])
{
	long fila=8000,columna=8000;
	double *matriz, *vector,*resultado;
	int i,j;

	matriz=malloc(fila*columna*sizeof(double));
	vector=malloc(columna*sizeof(double));
	resultado=malloc(fila*sizeof(double));

   	read_matriz(matriz,fila,columna);
   	read_vector(vector,columna);

	int thread_count= strtol(argv[1], NULL, 10); //Get number of threads from command line 

	clock_t begin = clock();
#	pragma omp parallel for num_threads(thread_count)\
	default(none) private(i,j) shared(matriz,vector,resultado,fila,columna)//shared todos los thread tienen eso
	for(i=0;i<fila;i++){
		resultado[i]=0;
		for(j=0;j<columna;j++){
			resultado[i]+=matriz[i*columna+j]*vector[j];
		}
	}

	clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("tiempo: %f\n", time_spent);
	return 0;
}