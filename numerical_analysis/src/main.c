#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

int main(void)
{
    int chunk = 1;
    int size = 500000;
    float* array = (float*)malloc(size * sizeof(float));

    if(!array)
    {
        printf("Failed to allocate array");
        exit(1);
    }

    double startTime = omp_get_wtime();

    #pragma omp parallel shared(array) num_threads(8)
    {
        #pragma omp single 
        printf("Number of threads: %d\n", omp_get_num_threads());

        printf("Hello from thread: %d\n", omp_get_thread_num());

        #pragma omp for schedule(static, chunk)
        for (int i = 0; i < size; ++i)
        {
            int n = 10001;
            float total = 0;
            for (int j = 0; j < n; ++j)
            {
                total += (float)j;
            }
            array[i] = (float)i * (2.0f * total / (float)n / (float)(n-1));
        }

        
    }

    double endTime = omp_get_wtime();

    printf("%f\n", array[size - 1]);

    /* for(int i = 0; i < size; ++i)
    {
        printf("%f\n", array[i]);
    } */

    printf("Time: %f\n", endTime - startTime);

    free(array);

    return 0;
}
