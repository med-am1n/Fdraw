#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// no need to include glfw3.h here
struct GLFWwindow;

namespace Gui 
{
    // call once before the rendering loop!
    inline void Init(GLFWwindow* window, const char* glsl_version = "#version 330") 
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
    inline void DrawMenu() 
    {
        ImGui::Begin("Menu");

        if (ImGui::Button("Button")) {
            // do somthing...
        }

        ImGui::Separator();

        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Application average %.1f FPS", io.Framerate);
        ImGui::Text("Frame time: %.3f ms/frame", 1000.0f / io.Framerate);

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
