#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"
#include "camera.h"
#include <stb_image.h>
#include <filesystem.h>
#include <gui.h>

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
GLFWwindow *startGLFW(int width, int height, const char *title, GLFWframebuffersizefun fb_cb, GLFWmousebuttonfun button_cb, GLFWcursorposfun mouse_cb, GLFWkeyfun key_cb);
unsigned int LoadTexture(const char *path);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool leftMouseDown = false;
double prevX, prevY;
bool hasPreviousPosition = false;
bool draw = true;
glm::vec2 selectionStart(0.0f);
glm::vec2 selectionEnd(0.0f);
glm::vec2 previousMousePos;
bool hasSelectedArea = false;

bool dragging = false;
bool selection = false;

glm::vec2 minSelectedArea(FLT_MAX);
glm::vec2 maxSelectedArea(-FLT_MAX);

struct Mesh
{
    unsigned int VAO;
    unsigned int VBO;
    std::vector<float> vertices;
};
Mesh CreateCircle(float radius, int res);

class Point
{
public:
    Mesh *mesh;

    glm::vec2 position;
    float radius;
    std::string type;
    bool selected = false;
    int strokId = -1;

    Point(Mesh *mesh, glm::vec2 position, float radius, std::string type)
    {
        this->mesh = mesh;
        this->position = position;
        this->radius = radius;
        this->type = type;
    }

    void Draw(Shader &shader)
    {
        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
        model = glm::scale(model, glm::vec3(radius, radius, 1.0f));
        shader.setMat4("model", model);

        glBindVertexArray(mesh->VAO);

        if (type == "circle")
        {
            glDrawArrays(GL_TRIANGLE_FAN, 0, mesh->vertices.size() / 3);
        }

        glBindVertexArray(0);
    }
};

Mesh circleMesh;

struct Stroke
{
    std::vector<Point> points;
    bool selected = false;
};

std::vector<Stroke> strokes;

int main()
{

    std::cout << "Hello, World!" << std::endl;

    glfwInit();

    GLFWwindow *window = startGLFW(SCR_WIDTH, SCR_HEIGHT, "Fdraw", framebuffer_size_callback, mouse_button_callback, cursor_position_callback, key_callback);
    if (!window)
        return -1;

    // initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // // initialize Imgui
    Gui::Init(window);

    // select rectangle
    unsigned int selectVBO, selectEBO, selectVAO;

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f};

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};
    unsigned int indexCount = 6;

    glGenVertexArrays(1, &selectVAO);
    glGenBuffers(1, &selectVBO);
    glGenBuffers(1, &selectEBO);

    glBindVertexArray(selectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, selectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // slected Area rectangle
    unsigned int selectedVAO, selectedVBO;
    glGenVertexArrays(1, &selectedVAO);
    glGenBuffers(1, &selectedVBO);
    glBindVertexArray(selectedVAO);
    glBindBuffer(GL_ARRAY_BUFFER, selectedVBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // build and compile the shader program
    Shader shader("../src/shaders/shader.vs", "../src/shaders/shader.fs");
    Shader selectShader("../src/shaders/selectShader.vs", "../src/shaders/selectShader.fs");
    Shader selectedShader("../src/shaders/selectedShader.vs", "../src/shaders/selectedShader.fs");
    circleMesh = CreateCircle(1.0f, 100);

    glEnable(GL_DEPTH_TEST);
    int currentStrokeId = 0;
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        processInput(window);

        // imgui
        Gui::BeginFrame();

        Gui::DrawMenu(draw);
        if (draw)
        {
            for (Stroke &s : strokes)
            {
                s.selected = false;
            }
            if (leftMouseDown)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                if (hasPreviousPosition)
                {
                    double dx = xpos - prevX;
                    double dy = ypos - prevY;
                    double distance = std::sqrt(dx * dx + dy * dy);
                    int numSamples = std::max(static_cast<int>(distance / 1.0f), 1);
                    Stroke stroke;
                    for (int i = 0; i <= numSamples; ++i)
                    {
                        float t = static_cast<float>(i) / numSamples;
                        float vx = prevX + (xpos - prevX) * t;
                        float vy = prevY + (ypos - prevY) * t;
                        Point circle(&circleMesh, glm::vec2(vx, vy), 1.0f, "circle");
                        circle.strokId = currentStrokeId;
                        strokes[strokes.size() - 1].points.push_back(circle);
                    }
                }

                prevX = xpos;
                prevY = ypos;
                hasPreviousPosition = true;
            }
            else
            {
                hasPreviousPosition = false;
            }
        }

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // rendering commands here
        glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
        shader.use();
        shader.setMat4("projection", projection);
        if (!draw)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            glm::vec2 mousePos = glm::vec2((float)xpos, (float)ypos);

            if (leftMouseDown && !selection)
            {
                if (!dragging)
                {
                    if (mousePos.x >= minSelectedArea.x &&
                        mousePos.x <= maxSelectedArea.x &&
                        mousePos.y >= minSelectedArea.y &&
                        mousePos.y <= maxSelectedArea.y)
                    {
                        std::cout << "you are in selected Area: " << xpos << ", " << ypos << std::endl;
                        dragging = true;
                        previousMousePos = mousePos;
                    }
                }
                if (dragging)
                {
                    if (leftMouseDown && !selection)
                    {
                        std::cout << "click in slected Area\n";
                        glm::vec2 delta = mousePos - previousMousePos;
                        for (Stroke &s : strokes)
                        {
                            if (s.selected == true)
                            {
                                for (Point &p : s.points)
                                {
                                    p.position += delta;
                                }
                            }
                        }
                        minSelectedArea += delta;
                        maxSelectedArea += delta;
                        previousMousePos = mousePos;
                    }
                }
            }

            if (leftMouseDown && !dragging)
            {
                selection = true;
                minSelectedArea = glm::vec2(FLT_MAX);
                maxSelectedArea = glm::vec2(-FLT_MAX);

                for (Stroke &s : strokes)
                {
                    s.selected = false;
                }

                glDisable(GL_DEPTH_TEST);
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                selectionEnd = glm::vec2(xpos, ypos);

                glm::vec2 center = (selectionStart + selectionEnd) * 0.5f;
                glm::vec2 size = glm::abs(selectionEnd - selectionStart);

                glm::vec2 minPos = glm::min(selectionStart, selectionEnd);
                glm::vec2 maxPos = glm::max(selectionStart, selectionEnd);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(center, 0.0f));
                model = glm::scale(model, glm::vec3(size, 1.0f));

                selectShader.use();
                selectShader.setMat4("model", model);
                selectShader.setMat4("projection", projection);

                glBindVertexArray(selectVAO);
                glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

                // border
                glLineWidth(2.0f);
                glDrawArrays(GL_LINE_LOOP, 0, 4);

                glBindVertexArray(0);
                for (Stroke &s : strokes)
                {
                    s.selected = true;

                    for (const Point &p : s.points)
                    {
                        glm::vec2 pos = p.position;

                        if (pos.x - p.radius < minPos.x ||
                            pos.x + p.radius > maxPos.x ||
                            pos.y - p.radius < minPos.y ||
                            pos.y + p.radius > maxPos.y)
                        {
                            s.selected = false;
                            break;
                        }
                    }
                    if (s.selected)
                    {
                        hasSelectedArea = true;
                        for (const Point &p : s.points)
                        {
                            minSelectedArea.x = std::min(minSelectedArea.x, p.position.x - p.radius);
                            minSelectedArea.y = std::min(minSelectedArea.y, p.position.y - p.radius);

                            maxSelectedArea.x = std::max(maxSelectedArea.x, p.position.x + p.radius);
                            maxSelectedArea.y = std::max(maxSelectedArea.y, p.position.y + p.radius);
                        }
                    }
                }
            }
        }

        if (hasSelectedArea)
        {
            glm::vec2 selectedAreaVertices[] = {
                {minSelectedArea.x, minSelectedArea.y},
                {maxSelectedArea.x, minSelectedArea.y},
                {maxSelectedArea.x, maxSelectedArea.y},
                {minSelectedArea.x, maxSelectedArea.y}};

            glBindVertexArray(selectedVAO);
            glBindBuffer(GL_ARRAY_BUFFER, selectedVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(selectedAreaVertices), selectedAreaVertices);
            glBindVertexArray(0);

            selectedShader.use();

            glm::mat4 model = glm::mat4(1.0f);

            selectedShader.setMat4("model", model);
            selectedShader.setMat4("projection", projection);

            glBindVertexArray(selectedVAO);

            glLineWidth(2.0f);

            glDrawArrays(GL_LINE_LOOP, 0, 4);

            glBindVertexArray(0);
        }

        for (Stroke &s : strokes)
        {
            for (Point &p : s.points)
            {
                shader.use();
                shader.setVec4("color", s.selected ? glm::vec4(1.0f, 0.3f, 0.3f, 1.0f)
                                                   : glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                p.Draw(shader);
            }
        }

        // imgui
        Gui::EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Gui::Shutdown();

    glfwTerminate();

    return 0;
}

GLFWwindow *startGLFW(int width, int height, const char *title,
                      GLFWframebuffersizefun fb_cb,
                      GLFWmousebuttonfun button_cb,
                      GLFWcursorposfun mouse_cb,
                      GLFWkeyfun key_cb)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, button_cb);
    glfwSetCursorPosCallback(window, mouse_cb);
    glfwSetKeyCallback(window, key_cb);

    return window;
}

unsigned int LoadTexture(const char *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format;

        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB;

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    windowWidth = width;
    windowHeight = height;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            leftMouseDown = true;
            double xpos, ypos;
            std::cout << "left mouse presed\n";
            glfwGetCursorPos(window, &xpos, &ypos);
            selectionStart = glm::vec2(xpos, ypos);
            selectionEnd = selectionStart;
            Stroke currentStroke;
            strokes.push_back(currentStroke);
        }
        else if (action == GLFW_RELEASE)
        {
            leftMouseDown = false;
            if (selection)
                selection = false;
            if (dragging)
                dragging = false;
        }
    }
}
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    // std::cout << "cursor position:( " << xpos << ", " << ypos << " )" << std::endl;
}
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    {
        std::cout << "key pressed: " << key << std::endl;
    }
}

Mesh CreateCircle(float radius, int res)
{

    Mesh mesh;

    // Generate vertices
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);

    for (int i = 0; i <= res; i++)
    {
        float angle = 2.0f * M_PI * i / res;

        mesh.vertices.push_back(radius * cos(angle));
        mesh.vertices.push_back(radius * sin(angle));
        mesh.vertices.push_back(0.0f);
    }

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    int stride = 3 * sizeof(float);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    return mesh;
}
