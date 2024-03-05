#pragma once

#include <stdio.h>

#include "vector.h"

struct mat3
{
    float elements[9];
};


static inline void matrix_transpose(float* mat, int side_size)
{
    for (int j = 0; j < side_size; ++j)
    {
        for (int i = j + 1; i < side_size; ++i)
        {
            float temp = mat[i + j * side_size];
            mat[i + j * side_size] = mat[j + i * side_size];
            mat[j + i * side_size] = temp;
        }
    }
}


static inline void matrix_multiply(float* result, float* left, float* right, int rows, int cols)
{
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
    matrix_transpose(transpose.elements, 3);
    return transpose;
}

static inline struct mat3 mat3_multiply(struct mat3 left, struct mat3 right)
{
    struct mat3 result;

    matrix_multiply(result.elements, left.elements, right.elements, 3, 3);

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
    matrix_multiply(result, left, right, 6, 6);
}

static inline void mat6_transpose(float* mat)
{
    matrix_transpose(mat, 6);
}

static inline void matrix_print(float* mat, int rows, int cols)
{
    for (int j = 0; j < rows; ++j)
    {
        for (int i = 0; i < cols; ++i)
        {
            printf("%2.f ", mat[i + j * cols]);
        }

        printf("\n");
    }

    printf("\n");
}

// Break a 6x6 matrix into its 3x3 quadrants
static inline void mat6_break_quads(float* mat, struct mat3* quads)
{
    printf("Breaking Quads\n\n");

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

        printf("Quad %i\n", q);
        matrix_print(quads[q].elements, 3, 3);
    }
}

// Join 3x3 quadrants into a single 6x6 matrix
static inline void mat6_join_quads(float* mat, struct mat3* quads)
{
    printf("Joining Quads\n\n");

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
