#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D
{
public:
    glm::vec2 position;
    float zoom;

    Camera2D(float x = 0.0f, float y = 0.0f);

    glm::mat4 GetViewMatrix() const;

    glm::mat4 GetProjectionMatrix(
        float windowWidth,
        float windowHeight) const;

    glm::vec2 ScreenToWorld(
        glm::vec2 screen,
        float windowWidth,
        float windowHeight) const;
};
