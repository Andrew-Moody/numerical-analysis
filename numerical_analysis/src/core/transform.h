#pragma once

#include <stdio.h>
#include <math.h>

struct vec4
{
    float x;
    float y;
    float z;
    float w;
};



struct mat4
{
    float e11, e12, e13, e14;
    float e21, e22, e23, e24;
    float e31, e32, e33, e34;
    float e41, e42, e43, e44;
};

static const struct mat4 MAT4_IDENTITY = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

static inline struct mat4 mat4_multiply(struct mat4 a, struct mat4 b)
{
    struct mat4 out;

    out.e11 = a.e11 * b.e11 + a.e12 * b.e21 + a.e13 * b.e31 + a.e14 * b.e41;
    out.e12 = a.e11 * b.e12 + a.e12 * b.e22 + a.e13 * b.e32 + a.e14 * b.e42;
    out.e13 = a.e11 * b.e13 + a.e12 * b.e23 + a.e13 * b.e33 + a.e14 * b.e43;
    out.e14 = a.e11 * b.e14 + a.e12 * b.e24 + a.e13 * b.e34 + a.e14 * b.e44;

    out.e21 = a.e21 * b.e11 + a.e22 * b.e21 + a.e23 * b.e31 + a.e24 * b.e41;
    out.e22 = a.e21 * b.e12 + a.e22 * b.e22 + a.e23 * b.e32 + a.e24 * b.e42;
    out.e23 = a.e21 * b.e13 + a.e22 * b.e23 + a.e23 * b.e33 + a.e24 * b.e43;
    out.e24 = a.e21 * b.e14 + a.e22 * b.e24 + a.e23 * b.e34 + a.e24 * b.e44;

    out.e31 = a.e31 * b.e11 + a.e32 * b.e21 + a.e33 * b.e31 + a.e34 * b.e41;
    out.e32 = a.e31 * b.e12 + a.e32 * b.e22 + a.e33 * b.e32 + a.e34 * b.e42;
    out.e33 = a.e31 * b.e13 + a.e32 * b.e23 + a.e33 * b.e33 + a.e34 * b.e43;
    out.e34 = a.e31 * b.e14 + a.e32 * b.e24 + a.e33 * b.e34 + a.e34 * b.e44;

    out.e41 = a.e41 * b.e11 + a.e42 * b.e21 + a.e43 * b.e31 + a.e44 * b.e41;
    out.e42 = a.e41 * b.e12 + a.e42 * b.e22 + a.e43 * b.e32 + a.e44 * b.e42;
    out.e43 = a.e41 * b.e13 + a.e42 * b.e23 + a.e43 * b.e33 + a.e44 * b.e43;
    out.e44 = a.e41 * b.e14 + a.e42 * b.e24 + a.e43 * b.e34 + a.e44 * b.e44;

    return out;
}

// Despite OpenGL and DirectX using different notational conventions in documentation
// the actual memory layout is the same. The difference seems to be more about how the
// memory is interpreted and how multiplications between matrices and vectors are performed
// DirectX seems to treat vectors as row vectors and use pre multiplication (based on mul() arg order)
// OpenGL seems to treat vectors as column vectors and use post multiplication

// the layout interpretation on the cpu side depends on how you define it and matrices can always be transposed
// before setting the uniform or use the flag to have OpenGL transpose for you (DirectX has this option as well)

// Row major feels more natural when interpreting the memory but treating vectors as columns is more common in math texts
// and leads to a more conventional multiplication order on the cpu

// I originally started defining the mat4 struct and operations before I remembered that OpenGL used different conventions
// from DirectX so thats why things are currently row major oriented and transforms must be composed S * R * T rather
// than T * R * S might have to swap around if it becomes a problem

// Row Major (DirectX)
// x.x x.y x.z 0
// y.x y.y y.z 0    => x.x x.y x.z 0 y.x y.y y.z 0 z.x z.y z.z 0 p.x p.y p.z 1
// z.x z.y z.z 0
// p.x p.y p.z 1

// Col Major (OpenGL)
// x.x y.x z.x p.x
// x.y y.y z.y p.y  => x.x x.y x.z 0 y.x y.y y.z 0 z.x z.y z.z 0 p.x p.y p.z 1
// x.z y.z z.z p.z
//   0   0   0   1


static inline struct mat4 mat4_rotation(float x, float y, float z)
{
    float ax = x * 0.017453f;
    float ay = y * 0.017453f;
    float az = z * 0.017453f;

    struct mat4 rotx = {
        1, 0, 0, 0,
        0, cosf(ax), sinf(ax), 0,
        0, -sinf(ax), cosf(ax), 0,
        0, 0, 0, 1

        /* 1, 0, 0, 0,
        0, cosf(ax), -sinf(ax), 0,
        0, sinf(ax), cosf(ax), 0,
        0, 0, 0, 1 */
    };


    struct mat4 roty = {
        cosf(ay), 0, -sinf(ay), 0,
        0, 1, 0, 0,
        sinf(ay), 0, cosf(ay), 0,
        0, 0, 0, 1

        /* cosf(ay), 0, sinf(ay), 0,
        0, 1, 0, 0,
        -sinf(ay), 0, cosf(ay), 0,
        0, 0, 0, 1 */
    };


    struct mat4 rotz = {
        cosf(az), sinf(az), 0, 0,
        -sinf(az), cosf(az), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1

        /* cosf(az), -sinf(az), 0, 0,
        sinf(az), cosf(az), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 */
    };

    // Order here was incorrect must be z * y * x not x * y * z
    /* struct mat4 rotxy = mat4_multiply(rotx, roty);
    struct mat4 rotation = mat4_multiply(rotxy, rotz); */

    struct mat4 rotzy = mat4_multiply(rotz, roty);
    struct mat4 rotation = mat4_multiply(rotzy, rotx);

    return rotation;
}

static inline struct mat4 mat4_translation(float x, float y, float z)
{
    struct mat4 translation = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1

        /* 1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1 */
    };

    return translation;
}

static inline struct mat4 mat4_scale(float x, float y, float z)
{
    struct mat4 scale = {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    };

    return scale;
}

static inline struct mat4 mat4_transform(float px, float py, float pz, float rx, float ry, float rz, float sx, float sy, float sz)
{
    struct mat4 translation = mat4_translation(px, py, pz);
    struct mat4 rotation = mat4_rotation(rx, ry, rz);
    struct mat4 scale = mat4_scale(sx, sy, sz);

    // Multiplication order is Scale * Rot * Trans because the multiplication
    // function is in terms of a row major matrix. May decide to change this around
    struct mat4 transform = mat4_multiply(rotation, translation);
    transform = mat4_multiply(scale, transform);

    return transform;
}

static inline void mat4_print(struct mat4 mat)
{
    printf("| %7.3f, %7.3f, %7.3f, %7.3f |\n| %7.3f, %7.3f, %7.3f, %7.3f |\n| %7.3f, %7.3f, %7.3f, %7.3f |\n| %7.3f, %7.3f, %7.3f, %7.3f |\n\n",
        mat.e11, mat.e12, mat.e13, mat.e14,
        mat.e21, mat.e22, mat.e23, mat.e24,
        mat.e31, mat.e32, mat.e33, mat.e34,
        mat.e41, mat.e42, mat.e43, mat.e44
    );
}