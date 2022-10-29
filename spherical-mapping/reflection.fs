#version 330 core
uniform sampler2D skybox;

in vec2 vN;

void main() {

  vec3 base = texture2D( skybox, vN ).rgb;
  gl_FragColor = vec4( base, 1. );

}