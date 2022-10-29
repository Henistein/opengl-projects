#version 330 core
out vec2 vN;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

  vec4 p = vec4( aPos, 1. );

  mat4 normal_matrix = transpose(inverse(model*view));
  vec3 e = normalize( vec3( model * view * p ) );
  vec3 n = normalize( vec3(normal_matrix * vec4(Normal, 1.0)));

  vec3 r = reflect( e, n );
  float m = 2. * sqrt(
    pow( r.x, 2. ) +
    pow( r.y, 2. ) +
    pow( r.z + 1., 2. )
  );
  vN = r.xy / m + .5;

  gl_Position = projection * model * view * p;

}