#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#include "graphics.h"
#include "fluid.h"
#include "frame.h"

int main(void)
{
    double startTime = omp_get_wtime();

    //finiteDifference1D();
    //finiteDifference2D();

    // Render a grid, or imported STL mesh
    //graphics_test();

    structural();

    double endTime = omp_get_wtime();

    printf("Time: %f\n", endTime - startTime);

    return 0;
}
