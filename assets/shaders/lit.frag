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
    vec3 tangent;
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
uniform int illuminationModel = 1;
uniform vec2 textureScale = vec2(1.0);  // Texture UV scaling from MTL -s option

// Normal mapping
uniform sampler2D normalMap;
uniform bool hasNormalMap = false;
uniform vec2 normalTextureScale = vec2(1.0);
uniform float bumpMultiplier = 1.0;

// PBR-style texture maps
uniform sampler2D specularMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;

uniform bool hasSpecularMap = false;
uniform bool hasRoughnessMap = false;
uniform bool hasAoMap = false;
uniform bool hasEmissiveMap = false;

// Fog properties
uniform vec3 fog_color = vec3(0.02, 0.02, 0.02);  // Dark bluish fog
uniform float fog_density = 0.01;
uniform float fog_start = 30.0;
uniform float fog_end = 100.0;

void main(){
    // Apply texture scaling to UV coordinates
    vec2 scaled_tex_coord = fs_in.tex_coord * textureScale;
    // Calculates the original color of the object pre lighting
    vec4 texture_color = tint * fs_in.color * texture(tex, scaled_tex_coord);
    // Apply Alpha thresholding
    if(texture_color.a < alphaThreshold) discard;
    
    // Calculate the normal - either from normal map or vertex normal
    vec3 normal = normalize(fs_in.normal);
    
    if (hasNormalMap) {
        // Construct TBN matrix for transforming normal map to world space
        vec3 T = normalize(fs_in.tangent);
        vec3 N = normal;
        // Re-orthogonalize T with respect to N (Gram-Schmidt)
        T = normalize(T - dot(T, N) * N);
        // Calculate bitangent - use cross(N, T) for OpenGL normal map convention
        vec3 B = cross(N, T);
        mat3 TBN = mat3(T, B, N);
        
        // Sample normal map with its own texture scale
        vec2 scaled_normal_coord = fs_in.tex_coord * normalTextureScale;
        vec3 normalMapSample = texture(normalMap, scaled_normal_coord).bgr;
        // Convert from [0,1] to [-1,1] range
        vec3 tangentNormal = normalMapSample * 2.0 - 1.0;
        // Apply bump multiplier to xy components
        tangentNormal.xy *= bumpMultiplier;
        tangentNormal = normalize(tangentNormal);
        // Transform to world space
        normal = normalize(TBN * tangentNormal);
    }
    
    // Sample material texture maps (fallback to uniforms if no map)
    vec3 material_specular = hasSpecularMap 
        ? texture(specularMap, scaled_tex_coord).rgb 
        : specular_color;
    
    // Roughness to shininess conversion: shininess = 2 / roughness^4 - 2
    // Inverse: roughness = pow(2 / (shininess + 2), 0.25)
    float material_shininess = hasRoughnessMap 
        ? (2.0 / pow(clamp(texture(roughnessMap, scaled_tex_coord).r, 0.001, 0.999), 4.0) - 2.0)
        : shininess;
    
    float material_ao = hasAoMap 
        ? texture(aoMap, scaled_tex_coord).r 
        : 1.0;
    
    vec3 material_emissive = hasEmissiveMap 
        ? texture(emissiveMap, scaled_tex_coord).rgb 
        : vec3(0.0);
    
    // Viewing direction is a ray from object to camera
    vec3 view_dir = normalize(camera_position-fs_in.world_position);
    
    // No ambient light - only flashlight illuminates (horror game atmosphere)
    // Start with emissive only (if present)
    vec3 result = material_emissive;
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
            // Hard cutoff - anything outside outer cone is pitch black
            if(theta < lights[i].outer_cone_angle) {
                continue;
            } else {
                // Smooth transition only between outer and inner cone
                float intensity = smoothstep(lights[i].outer_cone_angle, lights[i].inner_cone_angle, theta);
                attenuation *= intensity;
            }
        }
        }
        // Diffuse: using abs() for two-sided lighting (lights surfaces regardless of normal direction)
        float  diff = abs(dot(normal, light_direction));
        vec3 diffuse =diff * diffuse_color * texture_color.rgb * lights[i].color;
        
        // Specular - only if illuminationModel is 2 (full Blinn-Phong)
        vec3 specular = vec3(0.0);
        if (illuminationModel == 2) {
            vec3 halfway = normalize(light_direction+view_dir);
            float spec = pow(max(0,dot(halfway,normal)), material_shininess);
            specular = spec * material_specular * lights[i].color;
        }
        result += attenuation * (diffuse + specular);
    }
    
    // Calculate fog only if there's any light (avoid applying fog to pitch black areas)
    if (length(result) > 0.001) {
        float distance_to_camera = length(camera_position - fs_in.world_position);
        
        // Linear fog that increases between fog_start and fog_end
        float fog_factor = clamp((distance_to_camera - fog_start) / (fog_end - fog_start), 0.0, 1.0);
        
        // Scale fog to pitch black at far distances - matches light attenuation
        vec3 distance_scaled_fog = fog_color * (1.0 - fog_factor);
        
        // Mix result with distance-scaled fog
        result = mix(result, distance_scaled_fog, fog_factor);
    }
    
    // DEBUG: Uncomment ONE of these lines to visualize:
    // frag_color = vec4(normal * 0.5 + 0.5, 1.0);  // Visualize normals (should be consistent on parallel walls)
    // frag_color = vec4(abs(normal), 1.0);  // Visualize absolute normals
    // if (light_count > 0) frag_color = vec4(normalize(lights[0].direction) * 0.5 + 0.5, 1.0);  // Visualize light direction
    
    frag_color = vec4(result, texture_color.a);
}