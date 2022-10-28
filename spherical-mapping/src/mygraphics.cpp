#include "mygraphics.hpp"

void draw_cube(unsigned int *VBO, unsigned int *VAO){
  /* set up vertex data, buffers and cofigure vertex attributes */
  glGenVertexArrays(1, VAO);
  glGenBuffers(1, VBO);

  glBindVertexArray(*VAO);

  glBindBuffer(GL_ARRAY_BUFFER, *VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);

  // position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

  // texture coord attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

}

void load_texture(unsigned int *textureID, char const * path) {
    glGenTextures(1, textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data){
      GLenum format;
      if (nrComponents == 1)
          format = GL_RED;
      else if (nrComponents == 3)
          format = GL_RGB;
      else if (nrComponents == 4)
          format = GL_RGBA;

      glBindTexture(GL_TEXTURE_2D, *textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      stbi_image_free(data);
    }
    else{
      std::cout << "Texture failed to load at path: " << path << std::endl;
      stbi_image_free(data);
    }
}


/*
///////////////////////////////////////////////////////////////////////////////
// build vertices of sphere with smooth shading using parametric equation
// x = r * cos(u) * cos(v)
// y = r * cos(u) * sin(v)
// z = r * sin(u)
// where u: stack(latitude) angle (-90 <= u <= 90)
//       v: sector(longitude) angle (0 <= v <= 360)
///////////////////////////////////////////////////////////////////////////////
void build_sphere(unsigned int radius, float sector_count, float stack_count, unsigned int *VBO, unsigned int *VAO){
  vector<GLfloat> vertices;
  vector<GLuint> normals;
  vector<GLuint> text_coords;
  vector<unsigned int> indices;
  vector<unsigned int> line_indices;

  float x, y, z, xy;                          // vertex position
  float nx, ny, nz, length_inv = 1.0f/radius; // normal
  float s, t;                                 // text_coord

  float sector_step = 2 * PI / sector_count;
  float stack_step = PI / stack_count;
  float sector_angle, stack_angle;

  for(int i=0; i<=stack_count; i++){
    stack_angle = PI / 2 - i * stack_step; // starting from pi/2 to -pi/2
    xy = radius * cosf(stack_angle);       // r * cos(u)
    z = radius * sinf(stack_angle);        // r * sin(u)

    // add (sectorCount+1) vertices per stack
    // the first and last vertices have same position and normal, but different tex coords
    for(int j=0; j<=sector_count; j++){
      sector_angle = j * sector_step; // starting from 0 to 2pi

      // vertex position
      x = xy * cosf(sector_angle);    // r * cos(u) * cos(v)
      y = xy * sinf(sector_angle);    // r * cos(u) * sin(v)

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);

      // normalized vertex normal
      nx = x * length_inv;
      ny = y * length_inv;
      nz = z * length_inv;

      normals.push_back(nx);
      normals.push_back(ny);
      normals.push_back(nz);

      // vertex texture coords between [0,1]
      s = (float)j / sector_count;
      t = (float)i / stack_count;
      text_coords.push_back(s);
      text_coords.push_back(t);

    }
  }

  // indices
  //  k1--k1+1
  //  |  / |
  //  | /  |
  //  k2--k2+1
  unsigned int k1, k2;
  for (int i = 0; i < stack_count; i++){
    k1 = i * (sector_count + 1); // beginning of current stack
    k2 = k1 + sector_count + 1;  // beginning of next stack

    for (int j = 0; j < sector_count; j++, k1++, k2++){
      // 2 triangles per sector excluding 1st and last stacks
      if (i != 0){
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1+1);
      }

      if (i != (sector_count - 1)){
        indices.push_back(k1+1);
        indices.push_back(k2);
        indices.push_back(k2+1);
      }

      // vertical lines for all stacks
      line_indices.push_back(k1);
      line_indices.push_back(k2);
      if (i != 0){ // horizontal lines except 1st stack
        line_indices.push_back(k1);
        line_indices.push_back(k1 + 1);
      }
    }
  }
}

void draw_sphere(vector<>){
  // interleaved array
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(3, GL_FLOAT, interleavedStride, &interleavedVertices[0]);
  glNormalPointer(GL_FLOAT, interleavedStride, &interleavedVertices[3]);
  glTexCoordPointer(2, GL_FLOAT, interleavedStride, &interleavedVertices[6]);

  glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, indices.data());

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
*/