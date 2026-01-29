/**
 * @file math.cpp
 * @brief Implementation of mathematical utility functions
 */

#include "math.hpp"
#include "globals.hpp"

/**
 * @brief Convert a vec4 to a float array
 * 
 * Creates a stack-allocated array from the vector components.
 * Note: This returns a pointer to stack memory and should be used with caution.
 * 
 * @param v The vec4 to convert
 * @return float* Pointer to float array [x, y, z, w]
 */
float* tmt::math::vec4toArray(glm::vec4 v)
{
    float f[4] = {v.x, v.y, v.z, v.w};
    return f;
}

/**
 * @brief Convert a 4x4 matrix to a flat float array
 * 
 * Allocates a new array and copies all 16 matrix elements in column-major order.
 * Caller is responsible for deleting the returned array.
 * 
 * @param m The matrix to convert
 * @return float* Pointer to new float[16] array containing matrix elements
 */
float* tmt::math::mat4ToArray(glm::mat4 m)
{
    auto t = new float[16];
    int i = 0;
    // Iterate through columns, then rows (column-major order)
    for (int x = 0; x < 4; ++x)
    {
        for (int y = 0; y < 4; ++y)
        {
            t[i] = m[x][y];
            i++;
        }
    }

    return t;
}

/**
 * @brief Convert a 3x3 matrix to a float array
 * 
 * Currently unimplemented - returns nullptr.
 * TODO: Implement 3x3 matrix conversion if needed.
 * 
 * @param m The matrix to convert
 * @return float** Always returns nullptr
 */
float** tmt::math::mat3ToArray(glm::mat3 m)
{
    return nullptr;
}

/**
 * @brief Convert a vector of 4x4 matrices to an array of float pointers
 * 
 * Creates an array where each element points to the data of a matrix from the vector.
 * 
 * @param v Vector of matrices to convert
 * @return float** Pointer to array of float pointers
 */
float** tmt::math::mat4ArrayToArray(std::vector<glm::mat4> v)
{
    std::vector<float*> m4(v.size());
    int j = 0;
    for (int i = 0; i < v.size(); ++i)
    {
        m4[j] = value_ptr(v[i]);
    }

    return m4.data();
}

/**
 * @brief Create a rotation quaternion that makes an object look in a direction
 * 
 * Constructs a rotation from a forward direction and up vector.
 * This is useful for orienting cameras, characters, or any object that needs
 * to face a specific direction.
 * 
 * @param forward The direction to look towards (will be normalized)
 * @param up The up direction for orientation (will be normalized)
 * @return glm::quat Quaternion representing the look rotation
 */
glm::quat tmt::math::LookRotation(glm::vec3 forward, glm::vec3 up)
{
    glm::vec3 f = normalize(forward); // Ensure forward is normalized
    glm::vec3 r = normalize(cross(up, f)); // Right vector (perpendicular to up and forward)
    glm::vec3 u = cross(f, r); // Corrected up vector (perpendicular to forward and right)

    // Create rotation matrix from right, up, and forward vectors
    glm::mat3 rotationMatrix(r, u, f); // Right, Up, Forward form the basis vectors
    return quat_cast(rotationMatrix); // Convert rotation matrix to quaternion
}

/**
 * @brief Calculate the rotation from one direction to another
 * 
 * Computes the quaternion that rotates vector 'from' to align with vector 'to'.
 * Handles special cases:
 * - Vectors pointing in same direction (no rotation)
 * - Vectors pointing in opposite directions (180-degree rotation)
 * 
 * @param from Starting direction vector
 * @param to Target direction vector
 * @return glm::quat Quaternion that rotates from 'from' to 'to'
 */
glm::quat tmt::math::FromToRotation(glm::vec3 from, glm::vec3 to)
{
    glm::vec3 f = normalize(from);
    glm::vec3 t = normalize(to);

    float dotProduct = glm::dot(f, t);
    
    // If vectors are already aligned (dot product ~= 1), no rotation needed
    if (dotProduct >= 1.0f)
    {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion (no rotation)
    }
    
    // If vectors are opposite (dot product ~= -1), need 180-degree rotation
    if (dotProduct <= -1.0f)
    {
        // Find a perpendicular axis for 180-degree rotation
        glm::vec3 axis = normalize(cross(glm::vec3(1, 0, 0), f));
        if (glm::length(axis) < 0.001f)
        {
            // If cross with X-axis failed, try Y-axis
            axis = normalize(cross(glm::vec3(0, 1, 0), f));
        }
        return angleAxis(glm::pi<float>(), axis);
    }

    // Calculate rotation for general case
    glm::vec3 crossProduct = cross(f, t);
    float s = glm::sqrt((1 + dotProduct) * 2);
    float invS = 1 / s;

    return glm::quat(s * 0.5f, crossProduct.x * invS, crossProduct.y * invS, crossProduct.z * invS);
}

/**
 * @brief Spherical linear interpolation between two vectors
 * 
 * Performs smooth interpolation along the surface of a sphere.
 * This is useful for smooth camera rotations and directional transitions
 * where you want constant angular velocity.
 * 
 * @param start Starting vector (will be normalized)
 * @param end Ending vector (will be normalized)
 * @param t Interpolation factor (0.0 = start, 1.0 = end, values outside [0,1] extrapolate)
 * @return glm::vec3 Interpolated vector on the sphere surface
 */
glm::vec3 tmt::math::slerp(glm::vec3 start, glm::vec3 end, float t)
{
    // Normalize the input vectors to ensure they're on unit sphere
    glm::vec3 startNormalized = normalize(start);
    glm::vec3 endNormalized = normalize(end);

    // Compute the dot product (cosine of angle between vectors)
    float dot = glm::dot(startNormalized, endNormalized);

    // Clamp the dot product to avoid numerical errors with acos
    dot = glm::clamp(dot, -1.0f, 1.0f);

    // Calculate the angle between the vectors, scaled by t
    float theta = acos(dot) * t;

    // Compute a vector perpendicular to start in the plane of start and end
    glm::vec3 endPerpendicular = normalize(endNormalized - startNormalized * dot);

    // Perform spherical interpolation and return the result
    return startNormalized * cos(theta) + endPerpendicular * sin(theta);
}

/**
 * @brief Linear interpolation between two vectors
 * 
 * Simple component-wise linear interpolation.
 * For rotations, consider using slerp instead for smoother results.
 * 
 * @param start Starting vector
 * @param end Ending vector
 * @param t Interpolation factor (0.0 = start, 1.0 = end)
 * @return glm::vec3 Linearly interpolated vector
 */
glm::vec3 tmt::math::lerp(glm::vec3 start, glm::vec3 end, float t)
{
    return mix(start, end, t);
}

/**
 * @brief Linear interpolation between two float values
 * 
 * @param start Starting value
 * @param end Ending value
 * @param t Interpolation factor (0.0 = start, 1.0 = end)
 * @return float Linearly interpolated value
 */
float tmt::math::lerp(float start, float end, float t)
{
    return glm::lerp(start, end, t);
}

/**
 * @brief Calculate the magnitude (length) of a vector
 * 
 * Computes the Euclidean length of the vector using the formula:
 * magnitude = sqrt(x² + y² + z²)
 * 
 * @param v Vector to measure
 * @return float Length of the vector (always non-negative)
 */
float tmt::math::magnitude(glm::vec3 v)
{
    float m = glm::sqrt(glm::pow(v.x, 2) + glm::pow(v.y, 2) + glm::pow(v.z, 2));

    // Ensure magnitude is never negative (shouldn't happen, but safety check)
    if (m < 0)
    {
        m = 0;
    }

    return m;
}

/**
 * @brief Calculate the angle between two 2D vectors
 * 
 * Uses atan2 to compute the angle from vector 'b' to vector 'a'.
 * 
 * @param a First vector
 * @param b Second vector
 * @return float Angle in radians
 */
float tmt::math::angleBetween(glm::vec2 a, glm::vec2 b)
{
    var difference = a - b;

    var angle = std::atan2(difference.x, difference.y);

    return angle;
}
