#version 330 core

#define MAX_LIGHTS 8
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2



in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 world_position;
} fs_in;
out vec4 frag_color;

uniform vec4 tint;
uniform sampler2D tex;
uniform float alphaThreshold;
struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 attenuation;       
    //Necessary for spotlight
    float inner_cone_angle;
    float outer_cone_angle;
};
uniform int light_count;
uniform Light lights[MAX_LIGHTS];
uniform vec3 camera_position;

// Material properties (Blinn-Phong)
uniform vec3 ambient_color = vec3(0.1);
uniform vec3 diffuse_color = vec3(0.8);
uniform vec3 specular_color = vec3(0.5);
uniform float shininess = 32.0;

// Fog properties
uniform vec3 fog_color = vec3(0.02, 0.02, 0.03);  // Dark bluish fog
uniform float fog_density = 0.1;
uniform float fog_start = 5.0;
uniform float fog_end = 100.0;

void main(){
    // Calculates the original color of the object pre lighting
    vec4 texture_color = tint * fs_in.color * texture(tex, fs_in.tex_coord);
    // Apply Alpha thresholding
    if(texture_color.a < alphaThreshold) discard;
    // Viewing direction is a ray from object to camera
    vec3 view_dir = normalize(camera_position-fs_in.world_position);
    // Lighting according to Blinn-Phong Consists of 3 main part
    // Ambient, the innate light of the object or such:
    // Since it is mostly material dependent and doesn't change for diff types
    vec3 ambient = ambient_color * texture_color.rgb;
    vec3 result = ambient;
    vec3 normal = normalize(fs_in.normal);
    //Loop over all lights to add their effects to our rendered pixel
    for (int i = 0;i< light_count && i<MAX_LIGHTS;i++){
        vec3 light_direction;
        float attenuation = 1.0;
        if(lights[i].type == LIGHT_DIRECTIONAL){
            // We assume directional light like sun or such does not attenuate
            // Global Illumination as an example
            light_direction = -normalize(lights[i].direction);
        }
        else{
            //Handling spot/point light, we do physics calculations in world space
        vec3 to_light = lights[i].position - fs_in.world_position;
        float distance = length(to_light);
        light_direction = to_light/distance;
        attenuation = 1.0 / (lights[i].attenuation.x +
                             lights[i].attenuation.y * distance +
                             lights[i].attenuation.z * distance * distance);
        if(lights[i].type == LIGHT_SPOT){
            float theta = dot(light_direction,-normalize(lights[i].direction));
            // float epsilon = lights[i].inner_cone_angle-lights[i].outer_cone_angle;
            float intensity = smoothstep(lights[i].outer_cone_angle,lights[i].inner_cone_angle,theta);
            attenuation*=intensity;
        }
        }
        // Diffuse:
        float  diff = max(0.0,dot(normal,light_direction));
        vec3 diffuse =diff * diffuse_color * texture_color.rgb * lights[i].color;
        
        //specular
        vec3 halfway = normalize(light_direction+view_dir);
        float spec = pow(max(0,dot(halfway,normal)),shininess);
        vec3 specular = spec * specular_color * lights[i].color;
        result += attenuation * (diffuse + specular);
    }
    // Calculate fog
    float distance_to_camera = length(camera_position - fs_in.world_position);
    
    // Exponential fog (more realistic)
    float fog_factor = 1.0 - exp(-fog_density * distance_to_camera);
    // Mix result with fog
    result = mix(result, fog_color, fog_factor);
    
    frag_color = vec4(result, texture_color.a);
}