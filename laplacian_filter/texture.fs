#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int option;

const float offset = 1.0 / 300.0;

void main() {
  vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right
  );

  float kernel[9];
  if(option == 0){
    // horizontal
    kernel = float[](
        0,  1,  0,
        0, -2,  0,
        0,  1,  0
    );
  }
  else if(option == 1){
    // vertical 
    kernel = float[](
        0,  0,  0,
        1, -2,  1,
        0,  0,  0
    );
  }
  else if(option == 2){
    // both 
    kernel = float[](
        0,  1,  0,
        1, -4,  1,
        0,  1,  0
    );
  }
  else{
    // normal
    kernel = float[](
        0,  0,  0,
        0,  1,  0,
        0,  0,  0
    );
  }

  vec3 sampleTex[9];
  for(int i = 0; i < 9; i++)
  {
    sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
  }
  vec3 col = vec3(0.0);
  for(int i = 0; i < 9; i++)
    col += sampleTex[i] * kernel[i];

  FragColor = vec4(col, 1.0);
} 
