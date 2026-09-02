#ifndef CAMERA2D_H
#define CAMERA2D_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D
{
public:
    glm::vec2 Position;
    float Zoom;

    Camera2D(
        glm::vec2 position = glm::vec2(0.0f),
        float zoom = 1.0f
    )
        : Position(position), Zoom(zoom)
    {
    }

    glm::mat4 GetViewMatrix() const
    {
        glm::mat4 view(1.0f);
        view = glm::translate(view, glm::vec3(-Position, 0.0f));

        return view;
    }

    glm::mat4 GetProjectionMatrix(
        float windowWidth,
        float windowHeight
    ) const
    {
        float halfWidth = windowWidth / (2.0f * Zoom);
        float halfHeight = windowHeight / (2.0f * Zoom);
        
        return glm::ortho(-halfWidth, halfWidth, halfHeight, -halfHeight);
    }

    glm::vec2 ScreenToWorld(glm::vec2 screen, float windowWidth, float windowHeight) const
    {
        float x = Position.x +(screen.x - windowWidth * 0.5f) / Zoom;
        float y = Position.y +(screen.y - windowHeight * 0.5f) / Zoom;

        return glm::vec2(x, y);
    }
};

#endif
