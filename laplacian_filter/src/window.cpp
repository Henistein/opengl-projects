#include <iostream>
#include "window.hpp"

GLFWwindow *Window::get_window(void){
  return this->window;
}

void Window::init_glfw(void){
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  this->window = glfwCreateWindow(this->w_width, this->w_height, this->w_title.c_str(), NULL, NULL);
  if(this->window == NULL){
    perror("Failed window initialization");
    return;
  }

  glfwMakeContextCurrent(this->window);
}


int Window::init_glad(void){
  return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void Window::initialize(void){
  this->initialize(DEFAULT_W_WIDTH, DEFAULT_W_HEIGHT, DEFAULT_W_TITLE);
}

void Window::initialize(const unsigned width, const unsigned height, string title){
  this->w_width  = width;
  this->w_height = height;
  this->w_title  = title;
  this->init_glfw();
  this->init_glad();
}

void Window::run(void){
  while (!glfwWindowShouldClose(this->get_window())){
    this->refresh();
    glfwSwapBuffers(this->window);
    glfwPollEvents();
  }
}

void Window::finalize(void){
  glfwTerminate();
}

void Window::add_shader(Shader *shader){
  this->shader = shader;
}
