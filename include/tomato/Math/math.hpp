/**
 * @file math.hpp
 * @brief Mathematical utility functions and type conversions
 * 
 * This header provides math utilities including:
 * - Vector and matrix conversions between different libraries (GLM, Assimp, bx)
 * - Interpolation functions (lerp, slerp)
 * - Rotation calculations (quaternions, look-at)
 * - Bit packing/unpacking utilities
 */

#ifndef MATH_H
#define MATH_H

#include "utils.hpp"


namespace tmt::math
{

    // === Matrix and Vector Array Conversions ===
    
    /**
     * @brief Convert a vec4 to a float array
     * @param v The vec4 to convert
     * @return float* Pointer to float array [x, y, z, w]
     */
    float* vec4toArray(glm::vec4 v);

    /**
     * @brief Convert a 4x4 matrix to a float array
     * @param m The matrix to convert
     * @return float* Pointer to float array containing all 16 matrix elements
     */
    float* mat4ToArray(glm::mat4 m);

    /**
     * @brief Convert a 3x3 matrix to a float array
     * @param m The matrix to convert
     * @return float** Pointer to array of float pointers (currently unimplemented)
     */
    float** mat3ToArray(glm::mat3 m);

    /**
     * @brief Convert a vector of 4x4 matrices to an array of arrays
     * @param m Vector of matrices to convert
     * @return float** Pointer to array of float pointers
     */
    float** mat4ArrayToArray(std::vector<glm::mat4> m);

    // === Vector Conversions (GLM <-> Other Libraries) ===
    
    /**
     * @brief Convert GLM vec3 to bx::Vec3 (bgfx math library)
     * @param v GLM vector to convert
     * @return bx::Vec3 Converted vector for use with bgfx
     */
    inline bx::Vec3 convertVec3(glm::vec3 v)
    {
        return bx::Vec3{v.x, v.y, v.z};
    };

    /**
     * @brief Convert GLM vec3 to dynamically allocated float array
     * @param v Vector to convert
     * @return float* Pointer to new float[3] array (caller must delete[])
     */
    inline float* convertVec3A(glm::vec3 v)
    {
        var f = new float[3];
        f[0] = v[0];
        f[1] = v[1];
        f[2] = v[2];

        return f;
    }

    /**
     * @brief Convert GLM vec4 to dynamically allocated float array
     * @param v Vector to convert
     * @return float* Pointer to new float[4] array (caller must delete[])
     */
    inline float* convertVec4A(glm::vec4 v)
    {
        var f = new float[4];
        f[0] = v[0];
        f[1] = v[1];
        f[2] = v[2];
        f[3] = v[3];

        return f;
    }

    /**
     * @brief Convert float array to GLM vec3
     * @param arr Array of at least 3 floats
     * @return glm::vec3 Converted vector
     */
    inline glm::vec3 convertVec3(float* arr)
    {
        return glm::vec3{arr[0], arr[1], arr[2]};
    }

    /**
     * @brief Convert float array to GLM vec4
     * @param arr Array of floats
     * @param arrSize Number of elements to copy (max 4)
     * @return glm::vec4 Converted vector (remaining components set to 0)
     */
    inline glm::vec4 convertVec4(float* arr, int arrSize)
    {

        var v = glm::vec4();

        for (int i = 0; i < arrSize; ++i)
        {
            v[i] = arr[i];
        }

        return v;
    }

    /**
     * @brief Convert int array to GLM ivec4
     * @param arr Array of integers
     * @param arrSize Number of elements to copy (max 4)
     * @return glm::ivec4 Converted vector (remaining components set to -1)
     */
    inline glm::ivec4 convertIVec4(int* arr, int arrSize)
    {

        var v = glm::ivec4(-1);

        for (int i = 0; i < arrSize; ++i)
        {
            v[i] = arr[i];
        }

        return v;
    }

    /**
     * @brief Convert float array to GLM vec2
     * @param arr Array of at least 2 floats
     * @return glm::vec2 Converted 2D vector
     */
    inline glm::vec2 convertVec2(float* arr) { return glm::vec2{arr[0], arr[1]}; }

    /**
     * @brief Convert Assimp vector to GLM vec3
     * @param v Assimp aiVector3D
     * @return glm::vec3 Converted vector for use with GLM
     */
    inline glm::vec3 convertVec3(aiVector3D v)
    {
        return glm::vec3{v.x, v.y, v.z};
    };

    /**
     * @brief Convert Assimp quaternion to GLM quat
     * @param v Assimp aiQuaternion
     * @return glm::quat Converted quaternion (note: Assimp uses w,x,y,z order)
     */
    inline glm::quat convertQuat(aiQuaternion v)
    {
        return glm::quat{v.w, v.x, v.y, v.z};
    };

    /**
     * @brief Convert Assimp 4x4 matrix to GLM mat4
     * 
     * Performs proper row/column conversion between Assimp's matrix format
     * and GLM's column-major matrix format.
     * 
     * @param from Assimp aiMatrix4x4
     * @return glm::mat4 Converted matrix
     */
    inline glm::mat4 convertMat4(const aiMatrix4x4 from)
    {
        glm::mat4 to;
        // The a,b,c,d in Assimp is the row; the 1,2,3,4 is the column
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }

    /**
     * @brief Convert 2D float array to GLM mat4
     * @param from 4x4 float array
     * @return glm::mat4 Converted matrix
     */
    inline glm::mat4 convertMat4(float from[4][4])
    {
        glm::mat4 m;

        for (int x = 0; x < 4; ++x)
        {
            for (int y = 0; y < 4; ++y)
            {
                m[x][y] = from[x][y];
            }
        }

        return m;
    }

    // === Rotation and Orientation Functions ===
    
    /**
     * @brief Create a rotation quaternion that orients an object to look in a direction
     * 
     * @param forward The direction the object should face (will be normalized)
     * @param up The up direction (will be normalized)
     * @return glm::quat Rotation quaternion representing the orientation
     */
    glm::quat LookRotation(glm::vec3 forward, glm::vec3 up);;
    
    /**
     * @brief Calculate the rotation needed to rotate from one direction to another
     * 
     * @param from Starting direction vector
     * @param to Target direction vector
     * @return glm::quat Rotation quaternion that rotates from 'from' to 'to'
     */
    glm::quat FromToRotation(glm::vec3 from, glm::vec3 to);;

    // === Interpolation Functions ===
    
    /**
     * @brief Spherical linear interpolation between two vectors
     * 
     * Smoothly interpolates between two vectors along the surface of a sphere.
     * Useful for smooth rotations and camera movements.
     * 
     * @param start Starting vector
     * @param end Ending vector
     * @param t Interpolation factor (0.0 = start, 1.0 = end)
     * @return glm::vec3 Interpolated vector
     */
    glm::vec3 slerp(glm::vec3 start, glm::vec3 end, float t);

    /**
     * @brief Linear interpolation between two vectors
     * 
     * @param start Starting vector
     * @param end Ending vector
     * @param t Interpolation factor (0.0 = start, 1.0 = end)
     * @return glm::vec3 Interpolated vector
     */
    glm::vec3 lerp(glm::vec3 start, glm::vec3 end, float t);
    
    /**
     * @brief Linear interpolation between two floats
     * 
     * @param start Starting value
     * @param end Ending value
     * @param t Interpolation factor (0.0 = start, 1.0 = end)
     * @return float Interpolated value
     */
    float lerp(float start, float end, float t);

    // === Vector Math Utilities ===
    
    /**
     * @brief Calculate the magnitude (length) of a vector
     * 
     * @param v Vector to measure
     * @return float Length of the vector (always >= 0)
     */
    float magnitude(glm::vec3 v);

    /**
     * @brief Calculate the angle between two 2D vectors
     * 
     * @param a First vector
     * @param b Second vector
     * @return float Angle in radians
     */
    float angleBetween(glm::vec2 a, glm::vec2 b);

    // === Bit Manipulation Utilities ===
    
    /**
     * @brief Pack two 32-bit unsigned integers into one 64-bit integer
     * 
     * @param high Upper 32 bits
     * @param low Lower 32 bits
     * @return uint64_t Packed 64-bit value
     */
    inline uint64_t packU32ToU64(uint32_t high, uint32_t low) { return (static_cast<uint64_t>(high) << 32) | low; }

    /**
     * @brief Unpack a 64-bit integer into two 32-bit unsigned integers
     * 
     * @param packed The 64-bit value to unpack
     * @param high Output parameter for upper 32 bits
     * @param low Output parameter for lower 32 bits
     */
    inline void unpackU64ToU32(uint64_t packed, uint32_t& high, uint32_t& low)
    {
        high = static_cast<uint32_t>(packed >> 32); // Extract high 32 bits
        low = static_cast<uint32_t>(packed & 0xFFFFFFFF); // Extract low 32 bits
    }


}

#endif
