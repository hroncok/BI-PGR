#version 140

uniform mat4 Vmatrix;       // View                       --> world to eye coordinates
uniform sampler2D texSampler; // for texture access

smooth in vec2 texCoord_v;  // fragment texture coordinates

out vec4 color_f;

void main()
{
  vec3 normal = normalize((Vmatrix * vec4(0.0f, 1.0f, 0.0f, 0.0f)).xyz);
  vec3 global_ambient = vec3(0.6f);

  vec3 VMlightDirection =  normalize((Vmatrix *  vec4(0.0f, 1.0f, 1.0f, 0.0f)).xyz);
  float NdotL = max(0.0f, dot(normal, VMlightDirection));

  vec4 outputColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
  outputColor += vec4(vec3(NdotL), 0.0f);

  outputColor = outputColor * texture(texSampler, texCoord_v);

  color_f = outputColor;
}
