#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

const int NUM_OF_LIGHTS = 2;

float distance_from_sphere(vec3 p, vec3 c, float r) {
    return length(p - c) - r;
}

float distance_from_ellipsoid(vec3 p, vec3 c, vec3 r) {
    return (length((p - c)/r) - 1.0) * min(min(r.x,r.y),r.z);
}

float distance_from_torus(vec3 p, vec3 c, vec2 t) {
    vec2 q = vec2(length(p.xz - c.xz) - t.x, p.y - c.y);
    return length(q) - t.y;
}
float distance_from_plane(vec3 p, vec3 c, vec4 n) {
  // n must be normalized
  return dot(p - c, n.xyz) + n.w;
}

float distance_to_surface(vec3 p, int num) {
    if (num == 0) {
        return distance_from_sphere(p, vec3(0.0), 1.0);
    } else if (num == 1) {
        return distance_from_ellipsoid(p,  vec3(2.0, 0.0, -2.0), vec3(1.0, 0.5, 2.0));
    } else if (num == 2) {
        return distance_from_torus(p, vec3(-1.0, -3.0, 0.0), vec2(1.0, 0.5));
    } else if (num == 3) {
        return distance_from_plane(p, vec3(-1.0, -1.0, -1.0), vec4(0.0, 1.0, 0.0, 5.0));
    }
}

vec3 calculate_normal(vec3 p, int num)
{
    const vec3 small_step = vec3(0.001, 0.0, 0.0);
    vec3 normal = vec3(0.0);
    //gradients calculation
    normal.x = distance_to_surface(p + small_step.xyy, num) - distance_to_surface(p - small_step.xyy, num);
    normal.y = distance_to_surface(p + small_step.yxy, num) - distance_to_surface(p - small_step.yxy, num);
    normal.z = distance_to_surface(p + small_step.yyx, num) - distance_to_surface(p - small_step.yyx, num);

    return normalize(normal);
}

vec3 phong_light_model(vec3 k_diffuse, vec3 k_specular, float alpha, vec3 p, vec3 ray_pos,
                       vec3 light_position[NUM_OF_LIGHTS], vec3 light_intensity, int num) {
    vec3 final_color = vec3(0.0);
    vec3 normal = calculate_normal(p, num);
    vec3 viewer_direction = normalize(ray_pos - p);

    for (int i = 0; i < NUM_OF_LIGHTS; i++) {
        vec3 light_direction = normalize(light_position[i] - p);
        vec3 reflection_direction = normalize(reflect(-light_direction, normal));
        float dot_ln = dot(light_direction, normal);
        float dot_rv = dot(reflection_direction, viewer_direction);
        final_color += light_intensity * (k_diffuse * dot_ln + k_specular * pow(max(dot_rv, 0.0), alpha));
    }
    return final_color;
}

vec3 phong_illumination(vec3 k_ambient, vec3 k_diffuse, vec3 k_specular, float alpha, vec3 p,
                        vec3 ray_pos, vec3 light_position[NUM_OF_LIGHTS], int num) {
    vec3 ambient_light = 0.5 * vec3(1.0, 1.0, 1.0);
    vec3 color = ambient_light * k_ambient;
    vec3 light_intensity = vec3(0.45, 0.45, 0.45);

    color += phong_light_model(k_diffuse, k_specular, alpha, p, ray_pos,
                               light_position, light_intensity, num);
    return color;
}

vec3 ray_marching(vec3 ray_pos, vec3 ray_dir, vec3 light_position[NUM_OF_LIGHTS]) {
    float total_distance_traveled = 0.0;
    const int NUM_OF_STEPS = 175;
    const float MIN_HIT_DIST = 0.001;
    const float MAX_TRACE_DIST = 1000.0;
    vec3 final_color = vec3(0.0);

    for (int i = 0; i < NUM_OF_STEPS; i++) {
        vec3 current_position = ray_pos + total_distance_traveled * ray_dir;

        float distance_to_sphere = distance_to_surface(current_position, 0);
        float distance_to_ellipsoid = distance_to_surface(current_position, 1);
        float distance_to_torus = distance_to_surface(current_position, 2);
        float distance_to_plane = distance_to_surface(current_position, 3);

        if (distance_to_sphere < distance_to_ellipsoid &&
            distance_to_sphere < distance_to_torus &&
            distance_to_sphere < distance_to_plane) {
            if (distance_to_sphere < MIN_HIT_DIST) { //hit
                vec3 k_ambient = vec3(0.3, 0.15, 0.15);
                vec3 k_diffuse = vec3(0.7, 0.2, 0.2);
                vec3 k_specular = vec3(1.0, 1.0, 1.0);
                float shininess = 10.0;
                final_color += phong_illumination(k_ambient, k_diffuse, k_specular, shininess,
                                          current_position, ray_pos, light_position, 0);
                //final_colour += phong_illumination(k_ambient, k_diffuse, k_specular, shininess,
                                          //current_position, ray_pos, light_position2, 0);
                return final_color;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_sphere;
        } else if (distance_to_ellipsoid < distance_to_sphere &&
                   distance_to_ellipsoid < distance_to_torus &&
                   distance_to_ellipsoid  < distance_to_plane){
            if (distance_to_ellipsoid < MIN_HIT_DIST) { //hit
                vec3 k_ambient = vec3(0.15, 0.3, 0.15);
                vec3 k_diffuse = vec3(0.2, 0.7, 0.2);
                vec3 k_specular = vec3(1.0, 1.0, 1.0);
                float shininess = 10.0;
                final_color += phong_illumination(k_ambient, k_diffuse, k_specular, shininess,
                                          current_position, ray_pos, light_position, 1);
                return final_color;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_ellipsoid;
        } else if (distance_to_torus < distance_to_sphere &&
                   distance_to_torus < distance_to_ellipsoid &&
                   distance_to_torus  < distance_to_plane){
            if (distance_to_torus < MIN_HIT_DIST) {
                vec3 k_ambient = vec3(0.15, 0.3, 0.3);
                vec3 k_diffuse = vec3(0.2, 0.7, 0.7);
                vec3 k_specular = vec3(1.0, 1.0, 1.0);
                float shininess = 10.0;
                final_color += phong_illumination(k_ambient, k_diffuse, k_specular, shininess,
                                          current_position, ray_pos, light_position, 2);
                return final_color;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_torus;
        } else {
            if (distance_to_plane < MIN_HIT_DIST) {
                vec3 k_ambient = vec3(0.2, 0.2, 0.2);
                vec3 k_diffuse = vec3(0.7, 0.7, 0.7);
                vec3 k_specular = vec3(1.0, 1.0, 1.0);
                float shininess = 10.0;
                final_color += phong_illumination(k_ambient, k_diffuse, k_specular, shininess,
                                          current_position, ray_pos, light_position, 3);
                return final_color;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_plane;
        }
    }
    return vec3(0.0, 0.0, 0.1);
}

/**
TODO:
* add variables (global? passed to functions? defined?) for sphere and ellipsoid sizes
* same for coordinates, colors and light position, etc.
* instead of light_position 1 and 2 try to make array => change computing of color to cycle in phong_light_model

OBJECT 0: sphere
OBJECT 1: ellipsoid
OBJECT 2: torus
OBJECT 3: plane
OBJECT 4: ???
*/


void main() {
    vec2 tmp = 0.5 + vec2(fragmentTexCoord.x * g_screenWidth /2, fragmentTexCoord.y * g_screenHeight /2); //try to change
    vec3 ray_dir = normalize(vec3(tmp, -(g_screenWidth)/tan(3.14159265/3)));
    mat3 rdMat = mat3(g_rayMatrix);//multiply rayMatrix 3x3 x rd
    ray_dir = rdMat * ray_dir;
    vec3 ray_pos = vec3(0.0 + g_rayMatrix[3][0], 0.0 + g_rayMatrix[3][1], 5.0 + g_rayMatrix[3][2]); //+ rayMatrix[3][i]

    vec3 light_position[NUM_OF_LIGHTS];
    light_position[0] = vec3(2.0, 4.0, 4.0);
    light_position[1] = vec3(-7.0, 7.0, -10.0);
    vec3 shader_color = ray_marching(ray_pos, ray_dir, light_position);
    color = vec4(shader_color, 1.0);
}

