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

vec3 ray_marching(vec3 ro, vec3 rd) {
    float total_distance_traveled = 0.0;
    const int NUM_OF_STEPS = 128;
    const float MIN_HIT_DIST = 0.01;
    const float MAX_TRACE_DIST = 1000.0;
    const float radius = 1.0f;
    for (int i = 0; i < NUM_OF_STEPS; i++) {
        vec3 current_position = ro + total_distance_traveled * rd;
        float distance_to_closest = distance_from_ellipsoid(current_position, vec3(0.0), vec3(1.0, 2.0, 0.5));
        if (distance_to_closest < MIN_HIT_DIST) { //hit
            return vec3(1.0, 0.0, 0.0);
        }
        if (total_distance_traveled > MAX_TRACE_DIST) { //miss
            break;
        }
        total_distance_traveled += distance_to_closest;
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

    vec3 shader_color = ray_marching(ro, rd);
    color = vec4(shader_color, 1.0);
}

