#pragma once

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <shader_m.h>

using namespace std;

class Window{
private:
  GLFWwindow *window;

  /* Window specs */
  unsigned w_height;
  unsigned w_width;
  string w_title;

  /* Timing */
  float delta_time = 0.0f;
  float last_frame = 0.0f;
  float current_frame;

  //mouse_data mouse;
  //Camera camera;

  GLFWframebuffersizefun framebuffer_size_callback = nullptr;
  GLFWmousebuttonfun     mousebtn_callback         = nullptr;
  GLFWcursorposfun       mouse_callback            = nullptr;
  GLFWscrollfun          scroll_callback           = nullptr;

  /* Shaders */
  Shader* shader;

  void init_glfw(void);
  int  init_glad(void);

protected:
  const unsigned DEFAULT_W_HEIGHT = 800;
  const unsigned DEFAULT_W_WIDTH  = 600;
  const string   DEFAULT_W_TITLE  = "";

public:
  /* Initialization*/
  void initialize(void);
  void initialize(const unsigned, const unsigned, string);
  void finalize(void);
  void run(void);
  virtual void refresh(void);

  /* Shader */
  void add_shader(Shader*);

  Window(){};
  ~Window() {this->finalize();};

  /* Properties */
  GLFWwindow *get_window(void);
  Shader *get_shader(void);

};