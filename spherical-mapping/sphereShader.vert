#version 330
// Input vertex data, different for all executions of this shader.
attribute vec3 vertexPosition_modelspace;

// Output data ; will be interpolated for each fragment.
varying vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * model * view * vec4(vertexPosition_modelspace, 1);
}
