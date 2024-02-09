#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include "fluid.h"

#include "graphics.h"

int main(void)
{
    double startTime = omp_get_wtime();

    //finiteDifference1D();

    //finiteDifference2D();

    // Render a triangle to show everything was installed correctly
    graphics_test();

    double endTime = omp_get_wtime();

    printf("Time: %f\n", endTime - startTime);

    return 0;
}
