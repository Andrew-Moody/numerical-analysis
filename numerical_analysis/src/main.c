#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include "fluid.h"

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

    //finiteDifference1D();

    finiteDifference2D();

    double endTime = omp_get_wtime();

    printf("Time: %f\n", endTime - startTime);

    free(array);

    return 0;
}
