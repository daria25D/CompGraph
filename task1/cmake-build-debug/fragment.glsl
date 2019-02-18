#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

float distance_from_sphere(vec3 p, vec3 c, float r) {
    return length(p - c) - r;
}

float distance_from_ellipsoid(vec3 p, vec3 c, vec3 r) {
    return (length((p - c)/r) - 1.0) * min(min(r.x,r.y),r.z);
}

float distance_to_surface(vec3 p, int num) {
    if (num == 0 || num == 2) {
        float sphere = distance_from_sphere(p, vec3(0.0), 1.0);
        return sphere;
    } else if (num == 1) {
        float ellipsoid = distance_from_ellipsoid(p,  vec3(2.0, 2.0, 0.0), vec3(1.0, 0.5, 2.0));
        return ellipsoid;
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

vec3 ray_marching(vec3 ro, vec3 rd, vec3 light_position) {
    float total_distance_traveled = 0.0;
    const int NUM_OF_STEPS = 128;
    const float MIN_HIT_DIST = 0.01;
    const float MAX_TRACE_DIST = 1000.0;
    const float radius = 1.0f;
    for (int i = 0; i < NUM_OF_STEPS; i++) {
        vec3 current_position = ro + total_distance_traveled * rd;
        float distance_to_sphere = distance_from_sphere(current_position, vec3(0.0), radius);
        float distance_to_ellipsoid = distance_from_ellipsoid(current_position, vec3(2.0, 2.0, 0.0), vec3(1.0, 0.5, 2.0));
        if (distance_to_sphere < distance_to_ellipsoid) {
            if (distance_to_sphere < MIN_HIT_DIST) { //hit
                vec3 normal = calculate_normal(current_position, 0);
                vec3 direction_to_light = normalize(current_position - light_position);
                float diffuse_intensity = max(0.0, dot(normal, direction_to_light));
                return vec3(1.0, 0.0, 0.0) * diffuse_intensity;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_sphere;
        } else {
            if (distance_to_ellipsoid < MIN_HIT_DIST) { //hit
                vec3 normal = calculate_normal(current_position, 1);
                vec3 direction_to_light = normalize(current_position - light_position);
                float diffuse_intensity = max(0.0, dot(normal, direction_to_light));
                return vec3(0.0, 1.0, 0.0) * diffuse_intensity;
            }
            if (total_distance_traveled > MAX_TRACE_DIST) { //miss
                break;
            }
            total_distance_traveled += distance_to_ellipsoid;
        }
    }
    return vec3(0.0, 0.0, 0.0);
}

void main() {
    vec2 tmp = 0.5 + vec2(fragmentTexCoord.x * g_screenWidth /2, fragmentTexCoord.y * g_screenHeight /2); //try to change
    vec3 rd = normalize(vec3(tmp, -(g_screenWidth)/tan(3.14159265/3)));
    mat3 rdMat = mat3(g_rayMatrix);//multiply 3x3 x rd
    rd = rdMat * rd;
    //vec2 uv = fragCoord.xy * 2.0 - 1.0;
    vec3 ro = vec3(0.0 + g_rayMatrix[3][0], 0.0 + g_rayMatrix[3][1], 5.0 + g_rayMatrix[3][3]); //+ ray_matrix[3][i]
    //vec3 rd = vec3(uv, 1.0);
    vec3 light_position = vec3(2.0, 5.0, 3.0);
    vec3 shader_color = ray_marching(ro, rd, light_position);
    color = vec4(shader_color, 1.0);
}

