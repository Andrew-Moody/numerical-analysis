#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

struct mat3
{
    float elements[9];
};

struct mat6
{
    float elements[36];
};

struct Matrix
{
    float* elements;
    int rows;
    int cols;
};

static inline void matrix_init(struct Matrix* matrix, int rows, int cols, int initialize)
{
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->elements = malloc(sizeof(float) * rows * cols);

    if (initialize)
    {
        for (int i = 0; i < rows * cols; ++i)
        {
            matrix->elements[i] = 0;
        }
    }
}

static inline void matrix_release(struct Matrix* matrix)
{
    if (matrix)
    {
        free(matrix->elements);
        matrix->elements = NULL;
        matrix->rows = 0;
        matrix->cols = 0;
    }
}


static inline void matrix_copy(struct Matrix* copy, struct Matrix* original)
{
    matrix_init(copy, original->rows, original->cols, 0);
    for (int i = 0; i < original->rows * original->cols; ++i)
    {
        copy->elements[i] = original->elements[i];
    }
}

static inline void matrix_transpose_impl(float* matrix, int size)
{
    for (int j = 0; j < size; ++j)
    {
        for (int i = j + 1; i < size; ++i)
        {
            float temp = matrix[i + j * size];
            matrix[i + j * size] = matrix[j + i * size];
            matrix[j + i * size] = temp;
        }
    }
}

static inline void matrix_transpose(struct Matrix* matrix)
{
    matrix_transpose_impl(matrix->elements, matrix->rows);
}

// Helper function for matrix multiplication. Does no allocation
static inline void matrix_multiply_static(float* result, const float* left, const float* right, int rows, int cols)
{
    if (!result || !left || !right)
    {
        printf("matrix_multiply_static was passed one or more null pointers");
        return;
    }

    for (int j = 0; j < rows; ++j)
    {
        for (int i = 0; i < cols; ++i)
        {
            float v = 0;

            for (int k = 0; k < cols; ++k)
            {
                v += left[k + j * cols] * right[i + k * cols];
            }

            result[i + j * cols] = v;
        }
    }
}

static inline void matrix_multiply(struct Matrix* result, const struct Matrix left, const struct Matrix right)
{
    if (left.rows != right.cols || left.cols != right.rows)
    {
        printf("Matrix dimension mismatch Left is %dx%d and Right is %dx%d",
            left.rows, left.cols, right.rows, right.cols);
        return;
    }

    if (result->elements)
    {
        printf("Result Matrix is non empty");
        return;
    }

    matrix_init(result, left.rows, left.rows, 0);

    matrix_multiply_static(result->elements, left.elements, right.elements, left.rows, left.cols);
}


static inline void matrix_premultiply(float* result, float* matrix, float* vector, int rows, int cols)
{
    for (int j = 0; j < rows; ++j)
    {
        float v = 0;

        for (int i = 0; i < cols; ++i)
        {
            v += matrix[i + j * cols] * vector[i];
        }

        result[j] = v;
    }
}

static inline struct mat3 mat3_transpose(struct mat3 mat)
{
    struct mat3 transpose = mat;
    matrix_transpose_impl(transpose.elements, 3);
    return transpose;
}

static inline struct mat3 mat3_multiply(struct mat3 left, struct mat3 right)
{
    struct mat3 result;

    matrix_multiply_static(result.elements, left.elements, right.elements, 3, 3);

    return result;
}

static inline struct vec3 mat3_premultiply(struct mat3 mat, struct vec3 vec)
{
    struct vec3 result = {
        mat.elements[0] * vec.x + mat.elements[1] * vec.y + mat.elements[2] * vec.z,
        mat.elements[3] * vec.x + mat.elements[4] * vec.y + mat.elements[5] * vec.z,
        mat.elements[6] * vec.x + mat.elements[7] * vec.y + mat.elements[8] * vec.z
    };

    return result;
}

static inline void mat6_multiply(float* result, float* left, float* right)
{
    matrix_multiply_static(result, left, right, 6, 6);
}

static inline void mat6_transpose(struct mat6* matrix)
{
    matrix_transpose_impl(matrix->elements, 6);
}

static inline void mat6_print(struct mat6 matrix)
{
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            printf("%3.f ", matrix.elements[i + j * 6]);
        }

        printf("\n");
    }

    printf("\n");
}

static inline void matrix_print(const struct Matrix matrix)
{
    for (int j = 0; j < matrix.rows; ++j)
    {
        for (int i = 0; i < matrix.cols; ++i)
        {
            printf("%3.f ", matrix.elements[i + j * matrix.cols]);
        }

        printf("\n");
    }

    printf("\n");
}

// Break a 6x6 matrix into its 3x3 quadrants
static inline void mat6_break_quads(float* mat, struct mat3* quads)
{
    //printf("Breaking Quads\n\n");

    for (int q = 0; q < 4; ++q)
    {
        int offset = 3 * (q % 2) + 18 * (q / 2);

        for (int j = 0; j < 3; ++j)
        {
            for (int i = 0; i < 3; ++i)
            {
                quads[q].elements[i + 3 * j] = mat[offset + i + 6 * j];
            }
        }

        /* printf("Quad %i\n", q);
        matrix_print(quads[q].elements, 3, 3); */
    }
}

// Join 3x3 quadrants into a single 6x6 matrix
static inline void mat6_join_quads(float* mat, struct mat3* quads)
{
    //printf("Joining Quads\n\n");

    for (int q = 0; q < 4; ++q)
    {
        int offset = 3 * (q % 2) + 18 * (q / 2);

        for (int j = 0; j < 3; ++j)
        {
            for (int i = 0; i < 3; ++i)
            {
                mat[offset + i + 6 * j] = quads[q].elements[i + 3 * j];
            }
        }
    }
}
