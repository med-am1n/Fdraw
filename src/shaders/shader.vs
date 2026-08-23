#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform float aspectRatio;

void main()
{
	vec4 pos = model * vec4(aPos, 1.0);

    pos.x /= aspectRatio;

    gl_Position = pos;
}