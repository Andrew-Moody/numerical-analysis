#pragma once

#include <math.h>

struct vec3
{
    float x;
    float y;
    float z;
};

static inline struct vec3 vec3_scale(struct vec3 vec, float scalar)
{
    vec.x *= scalar;
    vec.y *= scalar;
    vec.z *= scalar;
    return vec;
}

static inline struct vec3 vec3_add(struct vec3 vec1, struct vec3 vec2)
{
    return (struct vec3) { vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z };
}

static inline struct vec3 vec3_subtract(struct vec3 vec1, struct vec3 vec2)
{
    return (struct vec3) { vec1.x - vec2.x, vec1.y - vec2.y, vec1.z - vec2.z };
}

static inline float vec3_square_length(struct vec3 vec)
{
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

static inline float vec3_length(struct vec3 vec)
{
    return sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

static inline float vec3_distance(struct vec3 vec1, struct vec3 vec2)
{
    struct vec3 diff = vec3_subtract(vec1, vec2);
    return vec3_length(diff);
}

static inline struct vec3 vec3_normalize(struct vec3 vec)
{
    float length = vec3_length(vec);

    // can't normalize if length is zero
    if (length == 0.0f)
    {
        return (struct vec3) { 0.0f, 0.0f, 0.0f };
    }

    return vec3_scale(vec, 1 / length);
}

static inline float vec3_dot(struct vec3 vec1, struct vec3 vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

static inline struct vec3 vec3_cross(struct vec3 vec1, struct vec3 vec2)
{
    struct vec3 cross = {
        (vec1.y * vec2.z) - (vec1.z * vec2.y),
        (vec1.z * vec2.x) - (vec1.x * vec2.z),
        (vec1.x * vec2.y) - (vec1.y * vec2.x)
    };

    return cross;
}


struct mat3
{
    float e11, e12, e13;
    float e21, e22, e23;
    float e31, e32, e33;

    float elements[36];
};

static inline struct mat3 mat3_transpose(struct mat3 mat)
{
    struct mat3 transpose = {
        mat.e11, mat.e21, mat.e31,
        mat.e12, mat.e22, mat.e32,
        mat.e13, mat.e23, mat.e33
    };

    return transpose;
}

/* static inline struct mat3 mat3_multiply(struct mat3 left, struct mat3 right)
{
    struct mat3 result = {
        left.e11 * right.e11 + left.e12 * right.e21 + left.e13 * right.e31,
        left.e11 * right.e12 + left.e12 * right.e22 + left.e13 * right.e32,
        left.e11 * right.e13 + left.e12 * right.e23 + left.e13 * right.e33,

        left.e21 * right.e11 + left.e22 * right.e21 + left.e23 * right.e31,
        left.e21 * right.e12 + left.e22 * right.e22 + left.e23 * right.e32,
        left.e21 * right.e13 + left.e22 * right.e23 + left.e23 * right.e33,

        left.e31 * right.e11 + left.e32 * right.e21 + left.e33 * right.e31,
        left.e31 * right.e12 + left.e32 * right.e22 + left.e33 * right.e32,
        left.e31 * right.e13 + left.e32 * right.e23 + left.e33 * right.e33,
    };

    return result;
} */


static inline void mat3_multiply(struct mat3* result, struct mat3 left, struct mat3 right)
{
    for (int j = 0; j < 3; ++j)
    {
        for (int i = 0; i < 3; ++i)
        {
            float v = 0;

            for (int k = 0; k < 3; ++k)
            {
                v += left.elements[k + j * 3] * right.elements[i + 3 * k];
            }

            result->elements[i + j * 3] = v;
        }
    }
}

static inline struct vec3 mat3_premultiply(struct mat3 mat, struct vec3 vec)
{
    struct vec3 result = {
        mat.e11 * vec.x + mat.e12 * vec.y + mat.e13 * vec.z,
        mat.e21 * vec.x + mat.e22 * vec.y + mat.e23 * vec.z,
        mat.e31 * vec.x + mat.e32 * vec.y + mat.e33 * vec.z,
    };

    return result;
}

static inline struct vec3 mat3_postmultiply(struct vec3 vec, struct mat3 mat)
{
    struct vec3 result = {
        mat.e11 * vec.x + mat.e21 * vec.y + mat.e31 * vec.z,
        mat.e12 * vec.x + mat.e22 * vec.y + mat.e32 * vec.z,
        mat.e13 * vec.x + mat.e23 * vec.y + mat.e33 * vec.z,
    };

    return result;
}


static inline void mat6_multiply(float* result, float* left, float* right)
{
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            float v = 0;

            for (int k = 0; k < 6; ++k)
            {
                v += left[k + j * 6] * right[i + 6 * k];
            }

            result[i + j * 6] = v;
        }
    }
}

static inline void mat6_transpose(float* mat)
{
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            float temp = mat[i + j * 6];
            mat[i + j * 6] = mat[j + i * 6];
            mat[j + i * 6] = temp;
        }
    }
}