#include "SDL.h"
#include "guill/application.h"
#include "guill/uiappfilter.h"
#include "imgui.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  guill::Application::Config app_config;
  app_config.window_name = "Simple App";
  app_config.suggested_width = 1024;
  app_config.suggested_height = 768;

  guill::UiAppFilter::Config config;
  config.imgui_callback = [](float, int, int) {
    ImGui::ShowDemoWindow();
    ImGui::Begin("Info");
    auto pos = ImGui::GetIO().MousePos;
    ImGui::Text("%f, %f", pos.x, pos.y);
    ImGui::End();
  };
#if __ANDROID__
  config.imgui_scale = 2.0f;
#endif

  guill::UiAppFilter::Apply(config, &app_config);

  guill::Application app(app_config);
  app.RunUntilQuit();

  return 0;
}
