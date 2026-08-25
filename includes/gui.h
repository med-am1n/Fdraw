#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <glm/gtc/type_ptr.hpp>


// no need to include glfw3.h here
struct GLFWwindow;

namespace Gui
{
    // call once before the rendering loop!
    inline void Init(GLFWwindow *window, const char *glsl_version = "#version 330")
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark(); // Default dark theme

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    // call at the start of every frame.
    inline void BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // custom menu
    inline void DrawMenu(bool &draw, float &radius, glm::vec4 &color)
    {
        ImGui::Begin("Menu");

        if (ImGui::Button("Change mode"))
        {
            draw = !draw;
        }

        ImGui::SameLine();

        ImGui::Text("Mode: %s", draw ? "Draw" : "Select");

        ImGui::SameLine();

        ImGui::SetNextItemWidth(100.0f);
        ImGui::SliderFloat("Radius", &radius, 1.0f, 30.0f);

        ImGui::SameLine();

        ImGui::ColorEdit4(
            "Color",
            glm::value_ptr(color),
            ImGuiColorEditFlags_NoInputs);

        ImGui::SameLine();

        ImGuiIO &io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);

        ImGui::SameLine();

        ImGui::Text("%.3f ms/frame", 1000.0f / io.Framerate);

        ImGui::End();
    }

    // call at the end of every frame
    inline void EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // call once after the rendering loop!
    inline void Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

#endif
