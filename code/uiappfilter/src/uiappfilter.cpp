
#include "guill/uiappfilter.h"

#include "SDL.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

//=============================================================================
//                                    Using                                   =
//=============================================================================
using guill::Application;
using guill::UiAppFilter::Config;

//=============================================================================
//                              Helper Functions                              =
//=============================================================================
namespace {
void HelperRenderCallback(SDL_Window *window, const Config &config) {
  int window_width, window_height;
  SDL_GetWindowSize(window, &window_width, &window_height);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame(window);
  ImGui::NewFrame();

  const int display_width = static_cast<int>(ImGui::GetIO().DisplaySize.x);
  const int display_height = static_cast<int>(ImGui::GetIO().DisplaySize.y);

  if (config.imgui_callback) {
    config.imgui_callback(config.imgui_scale, display_width, display_height);
  }

  glViewport(0, 0, display_width, display_height);
  if (config.extra_render_callback) {
    config.extra_render_callback(config.imgui_scale, display_width,
                                 display_height);
  }
  glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
}

void HelperEventCallback(const SDL_Event &event) {
  // I'd really rather not do this, but that's what the API looks like
  SDL_Event &non_const_event = const_cast<SDL_Event &>(event);
  ImGui_ImplSDL2_ProcessEvent(&non_const_event);
}

}  // namespace

//=============================================================================
//                               Implementation                               =
//=============================================================================

void guill::UiAppFilter::Apply(const Config &config,
                               Application::Config *app_config) {
  // Render (both imgui and general) callback
  app_config->render_callback = [config](SDL_Window *window) {
    HelperRenderCallback(window, config);
  };

  // Event callback
  app_config->event_callback = [config](const SDL_Event &event) {
    if (!config.event_callback || !config.event_callback(event)) {
      HelperEventCallback(event);
    }
  };

  // Tick callback
  app_config->tick_callback = config.tick_callback;

  // Init callback
  app_config->init_callback = [config](Application *app) {
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
#if (!__EMSCRIPTEN__ && !__ANDROID__)
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForOpenGL(&app->GetWindow(), app->GetGLContext());
    ImGui_ImplOpenGL3_Init(nullptr);
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(config.imgui_scale);
    // Make style consistent with viewports
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
#ifdef __ANDROID__
    style.TouchExtraPadding = ImVec2(4.0f, 4.0f);
#endif
    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = true;
    font_config.Name[0] = '\0';  // Use default
    font_config.SizePixels = 13.0f * config.imgui_scale;
    io.Fonts->AddFontDefault(&font_config);
  };

  app_config->shutdown_callback = [](Application *app) {
    (void)app;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  };
}
