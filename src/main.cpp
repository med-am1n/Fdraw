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

struct Mesh
{
    unsigned int VAO;
    unsigned int VBO;
    std::vector<float> vertices;
};
Mesh CreateCircle(float radius, int res);

class Object
{
public:
    Mesh *mesh;

    glm::vec2 position;
    float radius;
    std::string type;

    Object(Mesh *mesh, glm::vec2 position, float radius, std::string type)
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
        model = glm::scale(model, glm::vec3(radius / 0.1f));
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
std::vector<Object> objects;

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

    // // initialize Imgui
    Gui::Init(window);

    // build and compile the shader program
    Shader shader("../src/shaders/shader.vs", "../src/shaders/shader.fs");
    circleMesh = CreateCircle(1.0f, 100);

    glEnable(GL_DEPTH_TEST);

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

        Gui::DrawMenu();

    if (leftMouseDown)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (hasPreviousPosition)
        {
            double dx = xpos - prevX;
            double dy = ypos - prevY;
            double distance = std::sqrt(dx * dx + dy * dy);
            int numSamples = std::max(static_cast<int>(distance / 1.0f),1);
            for (int i = 0; i <= numSamples; ++i)
            {
                float t = static_cast<float>(i) / numSamples;
                float vx = prevX + (xpos - prevX) * t;
                float vy = prevY + (ypos - prevY) * t;
                Object circle(&circleMesh, glm::vec2(vx, vy), 0.5f, "circle");
                objects.push_back(circle);
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

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // rendering commands here
        glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
        shader.use();
        shader.setMat4("projection", projection);

        for (auto i{objects.size()}; i-- > 0;)
        {
            objects[i].Draw(shader);
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
        }
        else if (action == GLFW_RELEASE)
        {
            leftMouseDown = false;
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