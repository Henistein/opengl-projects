#include <iostream>

#include "window.hpp"
#include "Sphere.h"
#include "mygraphics.hpp"
#include "camera.h"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// specs
unsigned int W_WIDTH = 1366;
unsigned int W_HEIGHT = 768;

unsigned int cube_texture;
unsigned int sphere_texture;
unsigned int cube_VBO, cube_VAO;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

/* Callbacks */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// objs
//Model ourModel;


/* Main code */
void Window::refresh(void){
  // frame time logic
  float currentFrame = static_cast<float>(glfwGetTime());
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
  // input
  processInput(this->window);

  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

  glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)W_WIDTH / (float)W_HEIGHT, 0.1f, 100.0f);

  // activate shader
  this->shaders["cube"]->use();
  // projection view model
  this->shaders["cube"]->setMat4("model", model);
  this->shaders["cube"]->setMat4("view", view);
  this->shaders["cube"]->setMat4("projection", projection);
  this->shaders["cube"]->setVec3("cameraPos", camera.Position);

  /* CUBE */
  // render object
  //glBindVertexArray(cube_VAO);
  // draw cube
  //glDrawArrays(GL_TRIANGLES, 0, 36);
  Model ourModel("suzanne.obj");
  ourModel.Draw(*this->shaders["cube"]);

  /* Sphere */
  // objects
  Sphere sphere(50, 50, 50);
  this->shaders["sphere"]->use();
  view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
  // projection view model
  this->shaders["sphere"]->setMat4("projection", projection);
  this->shaders["sphere"]->setMat4("view", view);
  this->shaders["sphere"]->setMat4("model", model);
  // texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, sphere_texture);
  // draw sphere
  sphere.Draw();
  
  // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
  // -------------------------------------------------------------------------------
  glfwSwapBuffers(this->get_window());
  glfwPollEvents();
}

int main(void){
  Window window;

  // initialize window
  window.initialize(W_WIDTH, W_HEIGHT, "");

  // enable depth
  glEnable(GL_DEPTH_TEST);

  // callbacks
  glfwSetFramebufferSizeCallback(window.get_window(), framebuffer_size_callback);
  glfwSetCursorPosCallback(window.get_window(), mouse_callback);
  glfwSetScrollCallback(window.get_window(), scroll_callback);
  // tell GLFW to capture our mouse
  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // build and compile shader
  window.add_shader("cube", new Shader("reflection.vs", "reflection.fs"));
  window.add_shader("sphere", new Shader("sphere.vs", "sphere.fs"));

  // draw cube and create a texture
  draw_cube(&cube_VBO, &cube_VAO);
  load_texture(&sphere_texture, "unknown.png");
  //ourModel = new Model("suzanne.obj");

  // activate shader pass the texture to it
  window.get_shader("cube")->use();
  window.get_shader("cube")->setInt("skybox", 0);

  window.get_shader("sphere")->use();
  window.get_shader("sphere")->setInt("skybox", 0);

  // run
  window.run();

  glDeleteVertexArrays(1, &cube_VAO);
  glDeleteBuffers(1, &cube_VBO);

  return 0;
}


// Inputs and Callbacks
void processInput(GLFWwindow *window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);
  if (firstMouse){
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(static_cast<float>(yoffset));
}