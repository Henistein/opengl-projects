#include <iostream>

#include "window.hpp"
#include "mygraphics.hpp"
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// specs
unsigned int W_WIDTH = 1366;
unsigned int W_HEIGHT = 768;

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

unsigned int texture;
int numStrips;
int numTrisPerStrip;
// register VAO
GLuint terrainVAO, terrainVBO, terrainEBO;

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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST); 

  // activate shader
  this->shaders["shader"]->use();

  // model view projection
  glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
  glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  glm::mat4 projection = glm::perspective(glm::radians(fov), (float)W_WIDTH / (float)W_HEIGHT, 0.1f, 1000.0f);

  this->shaders["shader"]->setMat4("model", model);
  this->shaders["shader"]->setMat4("view", view);
  this->shaders["shader"]->setMat4("projection", projection);


  // bind Texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  glBindVertexArray(terrainVAO);
  //        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  for (unsigned strip = 0; strip < numStrips; strip++)
  {
    glDrawElements(GL_TRIANGLE_STRIP,                                           // primitive type
                   numTrisPerStrip + 2,                                         // number of indices to render
                   GL_UNSIGNED_INT,                                             // index data type
                   (void *)(sizeof(unsigned) * (numTrisPerStrip + 2) * strip)); // offset to starting index
  }

  // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
  // -------------------------------------------------------------------------------
  glfwSwapBuffers(this->get_window());
  glfwPollEvents();
}

int main(void){
  Window window;

  // initialize window
  window.initialize(W_WIDTH, W_HEIGHT, "");
  //glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);

  // callbacks
  glfwSetFramebufferSizeCallback(window.get_window(), framebuffer_size_callback);
  glfwSetCursorPosCallback(window.get_window(), mouse_callback);
  glfwSetScrollCallback(window.get_window(), scroll_callback);
  // tell GLFW to capture our mouse
  glfwSetInputMode(window.get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  stbi_set_flip_vertically_on_load(true);

  // load height map texture
  int width, height, nrChannels;
  unsigned char *data = stbi_load("assets/render_map.png",
                                  &width, &height, &nrChannels, 0);

  // vertex generation
  std::vector<float> vertices;
  float yScale = 64.0f / 256.0f, yShift = 16.0f;
  int rez = 1;
  unsigned bytePerPixel = nrChannels;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      unsigned char *pixelOffset = data + (j + width * i) * bytePerPixel;
      unsigned char y = pixelOffset[0];

      // vertex
      vertices.push_back(-height / 2.0f + height * i / (float)height); // vx
      vertices.push_back((int)y * yScale - yShift);                    // vy
      vertices.push_back(-width / 2.0f + width * j / (float)width);    // vz
    }
  }
  stbi_image_free(data);

  // index generation
  std::vector<unsigned int> indices;
  for (unsigned int i = 0; i < height - 1; i++){ // for each row a.k.a. each strip
    for (unsigned int j = 0; j < width; j++){ // for each column
      for (unsigned int k = 0; k < 2; k++){ // for each side of the strip
        indices.push_back(j + width * (i + k));
      }
    }
  }

  numStrips = (height-1);
  numTrisPerStrip = width*2-2;

  glGenVertexArrays(1, &terrainVAO);
  glBindVertexArray(terrainVAO);

  glGenBuffers(1, &terrainVBO);
  glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(float), // size of vertices buffer
               &vertices[0],                    // pointer to first element
               GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &terrainEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices.size() * sizeof(unsigned int), // size of indices buffer
               &indices[0],                           // pointer to first element
               GL_STATIC_DRAW);

  //load_texture(&texture, "assets/grass.jpg");
  load_texture(&texture, "assets/wall.jpg");

  // build and compile shader
  window.add_shader("shader", new Shader("shader.vs", "shader.fs"));

  window.get_shader("shader")->use();
  window.get_shader("shader")->setInt("texture1", 0);

  // run
  window.run();

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