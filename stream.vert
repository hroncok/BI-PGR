#version 140

uniform mat4 PVMmatrix;    // Projection * View * Model  --> model to clip coordinates
uniform float time;         // used for simulation of moving lights (such as sun)

in vec3 position;     // vertex position in world space
in vec2 texCoord;	    // incoming texture coordinates

smooth out vec2 texCoord_v;	// outgoing texture coordinates

void main()
{
  // vertex position after the projection (gl_Position is predefined output variable)
  gl_Position = PVMmatrix * vec4(position, 1);   // out:v vertex in clip coordinates

  // ======== BEGIN OF SOLUTION - TASK 2-2 ======== //
  // play with the x (u) coordinate to simulate the movement, you should use the time uniform
  vec2 offset = vec2(0.0f,time);
  // ========  END OF SOLUTION - TASK 2-2  ======== //

  // outputs entering the fragment shader
  texCoord_v = texCoord + offset;
}
