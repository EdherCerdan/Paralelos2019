#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int thread_count;
void odd_even_openmp(int *vector, int n){
    int phase, i, tmp;
    for(phase = 0; phase < n; phase++){
        if(phase % 2 == 0){
        	#pragma omp parallel for num_threads(thread_count) default(none) shared(vector,n) private(i,tmp)
                for(i=1; i<n; i+=2){
                    if(vector[i-1] > vector[i]){
                       tmp = vector[i-1];
                       vector[i-1] = vector[i];
                       vector[i] = tmp;
                    }
                }
        }
        else{
            #pragma omp parallel for num_threads(thread_count) default(none) shared(vector,n) private(i,tmp)
                for(i=1; i<n-1; i+=2){
                    if (vector[i] > vector[i+1]){
                       tmp = vector[i+1];
                       vector[i+1] = vector[i];
                       vector[i] = tmp;
                    }
                }
        }
    }
}

int main(int argc, char* argv[]) {
    int  n;
    int* vector;
    double inicio, final;
    int thread_count= strtol(argv[1], NULL, 10); //Get number of threads from command line 

    n = 20000;
    vector = malloc(n*sizeof(int));
   
    int i;
    srand(time(NULL));
    for(i=0; i<n; i++){
        vector[i] = rand()%n;
    }

    inicio = omp_get_wtime();
    odd_even_openmp(vector,n);
    final = omp_get_wtime();

    printf("tiempo = %.6lf \n", final-inicio);

    free(vector);
    return 0;
}