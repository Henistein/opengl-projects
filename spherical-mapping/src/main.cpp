#include <iostream>

#include "window.hpp"
#include "Sphere.h"
#include "mygraphics.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// specs
unsigned int W_WIDTH = 1366;
unsigned int W_HEIGHT = 768;

unsigned int cube_texture;
unsigned int cube_VBO, cube_VAO;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

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

  glm::mat4 projection = glm::perspective(glm::radians(fov), (float)W_WIDTH / (float)W_HEIGHT, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

  /* CUBE */
  // render container
  glBindVertexArray(cube_VAO);
  // bind cube Texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture); 
  // activate shader
  this->shaders["cube"]->use();
  // projection view model
  this->shaders["cube"]->setMat4("projection", projection);
  this->shaders["cube"]->setMat4("view", view);
  this->shaders["cube"]->setMat4("model", model);
  // draw cube
  glDrawArrays(GL_TRIANGLES, 0, 36);

  /* Sphere */
  // objects
  Sphere sphere(50, 50, 50);
  this->shaders["sphere"]->use();
  // projection view model
  this->shaders["sphere"]->setMat4("projection", projection);
  this->shaders["sphere"]->setMat4("view", view);
  this->shaders["sphere"]->setMat4("model", model);
  // texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, cube_texture);
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
  window.add_shader("cube", new Shader("4.1.texture.vs", "4.1.texture.fs"));
  window.add_shader("sphere", new Shader("4.1.texture.vs", "4.1.texture.fs"));

  // draw cube and create a texture
  draw_cube(&cube_VBO, &cube_VAO);
  load_texture(&cube_texture, "wall.jpg");
  
  //create_texture(&cube_texture, "wall.jpg");

  // draw sphere (radius, sectors, stacks)
  //draw_sphere(2, 50, 50);
  //esfera(2);

  // activate shader pass the texture to it
  //window.get_shader("cube")->use();
  //window.get_shader("cube")->setInt("texture", cube_texture);

  // run
  window.run();

  glDeleteVertexArrays(1, &cube_VAO);
  glDeleteBuffers(1, &cube_VBO);

  return 0;
}


// Inputs and Callbacks
void processInput(GLFWwindow *window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }

  float cameraSpeed = static_cast<float>(2.5 * deltaTime);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){
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

  float sensitivity = 0.1f; // change this value to your liking
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // make sure that when pitch is out of bounds, screen doesn't get flipped
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}