#pragma once

#include <stdlib.h>
#include <math.h>

struct vecf
{
    float* elements;
    int count;
};

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

static inline void vecf_init(struct vecf* vec, int count)
{
    vec->count = count;
    vec->elements = malloc(sizeof(*vec->elements) * count);
}

static inline void vecf_release(struct vecf* vec)
{
    free(vec->elements);
    vec->elements = NULL;
    vec->count = 0;
}

static inline void vecf_fill(struct vecf* vec, float value)
{
    for (int i = 0; i < vec->count; ++i)
    {
        vec->elements[i] = value;
    }
}
