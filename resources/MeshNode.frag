#version 130

struct Material {
   vec3  ambient;
   vec3  diffuse;
   vec3  specular;
   float shininess;
};

struct Light {
   vec3  ambient;
   vec3  diffuse;
   vec3  specular;
   vec3  position;  // should be vec4, while position[3] = 0 for vector and 1 for position
   vec3  spotDirection;
   float spotCosCutoff;
   float spotExponent;
};

Light light;
uniform Light lights[1];

Light reflight = lights[0];

smooth in vec3 normal_v;    // camera space normal
smooth in vec3 position_v;  // camera space fragment position
noperspective in vec3 reflectDir;
uniform samplerCube cubeMapTex;
uniform float reflectFactor;


uniform float     time;         // used for simulation of moving lights (such as sun)

uniform mat4      Vmatrix;      // View                       --> world to eye coordinates

uniform Material material;  // material of this vertex

uniform sampler2D texSampler;		// texture sampler
smooth in vec2 texCoord_v;			// fragment texture coordinates

uniform bool       useTexture;

out vec4 outputColor;
out vec4 color_f;

vec4 spotLight(Light light, Material material, vec3 position, vec3 normal)
{
  vec3 ret = vec3(0.0f);

  // ======== BEGIN OF SOLUTION - TASK 2-1 ======== //
  // use the material and light structures to obtain the surface and light properties 
  // the position and normal variables contain transformed surface position and normal 
  // store the ambient, diffuse and specular terms to the ret variable 
  // for spot lights, light.position contains the light position 
  vec3 lightDirection = normalize(light.position - position);
  if (dot(normalize(light.spotDirection), -lightDirection) >= light.spotCosCutoff) { // if the light should be there
    vec3 ambient = material.ambient * light.ambient;
    vec3 diffuse = max(dot(normal, lightDirection),0.0f) * material.diffuse * light.diffuse;
    vec3 specular = material.specular * light.specular * pow(max(0.0f, dot(reflect(normal, lightDirection), -normalize(position))), material.shininess);
    ret = (ambient+diffuse+specular) * pow(max(dot(normalize(light.spotDirection), -lightDirection),0.0f), light.spotExponent);
  }
  // ========  END OF SOLUTION - TASK 2-1  ======== //
  return vec4(ret, 1.0f);
}

vec4 directionalLight(Light light, Material material, vec3 position, vec3 normal)
{
  vec3 ret = vec3(0.0f);

  // ======== BEGIN OF SOLUTION - TASK 1-1 ======== //
  // use the material and light structures to obtain the surface and light properties 
  // the position and normal variables contain transformed surface position and normal 
  // store the ambient, diffuse and specular terms to the ret variable 
  // glsl provides some built-in functions, for example: reflect, normalize, pow, dot 
  // for directional lights, light.position contains the direction 

  ret += max(dot(normalize(normal),normalize(-light.position)),0f) * material.diffuse * light.diffuse;
  ret += material.ambient * light.ambient;
  
  
  vec3 ref = reflect(normalize(-light.position),normalize(normal));
  float mycos = max(dot(ref,normalize(-position)),0f);
  ret += pow(mycos,material.shininess) * material.specular * light.specular;
  // ========  END OF SOLUTION - TASK 1-1  ======== //

  return vec4(ret, 1.0f);
}

void main()
{
  vec3 normal = normalize(normal_v);
  vec3 global_ambient = vec3(0.4f);
  float sunSpeed = 0.2f;

  Light sun;
  //sun.ambient = vec3(0.0f);
  sun.diffuse = vec3(1.0f);
  sun.specular = vec3(1.0f);

  // Disco!
  if (int(floor(time*5)) % 2 == 0) reflight.diffuse = vec3(1.0f,0.0f,0.0f);

  // ======== BEGIN OF SOLUTION - TASK 1-2 ======== //
  // calculate correct direction to the sun using the time and sunSpeed variables
  // dont forget to translate the direction to the view coordinates (using Vmatrix)
   vec4 four = vec4(sin(time*sunSpeed), cos(time*sunSpeed), 0.0f, 0.0f) * Vmatrix;
   sun.ambient = vec3(abs(cos(time*sunSpeed)));
   sun.position = four.xyz;
  // ========  END OF SOLUTION - TASK 1-2  ======== //

  // initialize the output with the ambient term
  vec4 outputColor = vec4(material.ambient * global_ambient, 0.0f);
  // accumulate contributions from all lights
  outputColor += directionalLight(sun, material, position_v, normal);
  outputColor += spotLight(reflight, material, position_v, normal);

  if(useTexture)
    outputColor =  outputColor * texture(texSampler, texCoord_v);
  
  vec4 cubeMapColor = texture(cubeMapTex, reflectDir);
  outputColor = mix(outputColor, cubeMapColor, reflectFactor);

  color_f = outputColor;
}
