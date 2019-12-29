#include "guill/application.h"

#include "SDL.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  guill::Application::Config config;
  config.window_name = "Simple App";
  config.suggested_width = 200;
  config.suggested_height = 200;

  guill::Application app(config);
  app.RunUntilQuit();
  return 0;
}
