#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

const int NUM_OF_LIGHTS = 2;

//objects
float sphere; //current distance
vec3  sphere_centre    = vec3(0.0, 0.0, 0.0);
float sphere_radius    = 1.0;
vec3  sphere_color     = vec3(0.9, 0.1, 0.1);
float sphere_shininess = 10.0;

float ellipsiod;
vec3  ellipsoid_centre     = vec3(2.0, 0.0, -2.0);
vec3  ellipsoid_dimensions = vec3(1.0, 0.5, 2.0);
vec3  ellipsoid_color      = vec3(0.0, 0.7, 0.0);
float ellipsod_shininess   = 70.0;

float torus;
vec3  torus_centre     = vec3(-1.0, -3.0, 0.0);
vec2  torus_dimensions = vec2(1.0, 0.5);
vec3  torus_color      = vec3(0.2, 0.7, 0.7);
float torus_shininess  = 120.0;

float plane;
vec3  plane_centre    = vec3(-1.0, -1.0, -1.0);
vec4  plane_normal    = vec4(0.0, 1.0, 0.0, 5.0);
vec3  plane_color     = vec3(0.8);
float plane_shininess = 50.0;

float cube_for_sphere;
vec3  cube_for_sphere_centre     = vec3(0.0);
vec3  cube_for_sphere_dimensions = vec3(1.0);

float shininess;

//distance functions
float distance_from_sphere(vec3 p, vec3 c, float r) {
    return length(p - c) - r;
}
float distance_from_ellipsoid(vec3 p, vec3 c, vec3 r) {
    return (length((p - c)/r) - 1.0) * min(min(r.x,r.y),r.z);
}
float distance_from_torus(vec3 p, vec2 t) {
    vec2 q = vec2(length(p.xz) - t.x, p.y);
    return length(q) - t.y;
}
float distance_from_plane(vec3 p, vec3 c, vec4 n) {
  // n must be normalized
  return dot(p - c, n.xyz) + n.w;
}
float distance_from_cube(vec3 p, vec3 c, vec3 s) {
    vec3 d = abs(p - c) - s;

    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    float outsideDistance = length(max(d, 0.0));

    return insideDistance + outsideDistance;
}
//changing the final shape
float intersect(float dist_a, float dist_b) {
    return max(dist_a, dist_b);
}
float twist_torus(vec3 p, vec3 c, vec2 t, float twists) {
    float k = cos(twists*(p.y - c.y));
    float s = sin(twists*(p.y - c.y));
    mat2  m = mat2(k, -s, s, k);
    vec3  q = vec3(m*(p.xz - c.xz), p.y - c.y);
    return distance_from_torus(q, t);
}
float bend_torus(vec3 p, vec3 c, vec2 t, float bends) {
    float k = cos(bends*(p.y - c.y));
    float s = sin(bends*(p.y - c.y));
    mat2  m = mat2(k, -s, s, k);
    vec3  q = vec3(m*(p.xy - c.xy), (p.z - c.z));
    return distance_from_torus(q, t);
}
//distance to a surface specified
float distance_to_surface(vec3 p, int num) {
    if (num == 0) {
        //return distance_from_sphere(p, vec3(0.0), 1.0);
        return intersect(distance_from_sphere(p / 1.2, sphere_centre, sphere_radius) * 1.2,
                         distance_from_cube(p, cube_for_sphere_centre, cube_for_sphere_dimensions));
    } else if (num == 1) {
        return distance_from_ellipsoid(p,  ellipsoid_centre, ellipsoid_dimensions);
    } else if (num == 2) {
        return bend_torus(p, torus_centre, torus_dimensions, 0);
    } else if (num == 3) {
        return distance_from_plane(p, plane_centre, plane_normal);
    }
}
//minimal distance to any surface
float min_distance(vec3 p) {
    sphere = distance_to_surface(p, 0);
    ellipsiod = distance_to_surface(p, 1);
    torus = distance_to_surface(p, 2);
    plane = distance_to_surface(p, 3);

    return min(min(sphere, ellipsiod), min(torus, plane));
}
//normal for a given point
vec3 calculate_normal(vec3 p) {
    const vec3 small_step = vec3(0.001, 0.0, 0.0);
    vec3 normal = vec3(0.0);
    //gradients calculation
    normal.x = min_distance(p + small_step.xyy) - min_distance(p - small_step.xyy);
    normal.y = min_distance(p + small_step.yxy) - min_distance(p - small_step.yxy);
    normal.z = min_distance(p + small_step.yyx) - min_distance(p - small_step.yyx);

    return normalize(normal);
}

vec3 color_of_closest_object(vec3 p) {
    float d = 1e10;
    vec3 color = vec3(0.0);
    if (sphere < d) {
        d = sphere;
        color = sphere_color;
        shininess = sphere_shininess;
    }
    if (ellipsiod < d) {
        d = ellipsiod;
        color = ellipsoid_color;
        shininess = ellipsod_shininess;
    }
    if (torus < d) {
        d = torus;
        color = torus_color;
        shininess = torus_shininess;
    }
    if (plane < d) {
        d = plane;
        color = plane_color;
        shininess = plane_shininess;
    }
    return color;
}
//to prevent self-illumination
vec3 color_lerp(vec3 p) {
    vec3 color = vec3(0.0);

    float sphere_weight = 0.0;
    float ellipsiod_weight = 0.0;
    float torus_weight = 0.0;
    float plane_weight = 0.0;

    float eps = 0.01;
    float GI = 0.7;
    if (sphere > eps) sphere_weight = GI/(sphere + 1.0);
    if (ellipsiod > eps) ellipsiod_weight = GI/(ellipsiod + 1.0);
    if (torus > eps) torus_weight = GI/(torus + 1.0);
    if (plane > eps) plane_weight = GI/(plane + 1.0);

    color = sphere_color * sphere_weight +
            ellipsoid_color * ellipsiod_weight +
            torus_color * torus_weight +
            plane_color * plane_weight;
    return color * color * color * 0.15;
}

vec3 intersection_point(vec3 ray_pos, vec3 ray_dir) {
    float d, t = 0.5;
    for (int i = 0; i < 175; i++) {
        d = min_distance(ray_pos + t*ray_dir);
        if (abs(d) < 0.01) return ray_pos + t*ray_dir;
        t += d;
    }
    return ray_pos + t * ray_dir;
}

float ambient_occlusion(vec3 p, vec3 n){
    const int steps = 12;
    const float delta = 0.5;

    float a = 0.0;
    float weight = 0.75;
    float m;
    for(int i = 1; i <= steps; i++) {
        float d = (float(i) / float(steps)) * delta;
        a += weight*(d - min_distance(p + n*d));
        weight *= 0.5;
    }
    return clamp(1.0 - a, 0.0, 1.0);
}

float soft_shadow(vec3 ray_pos, vec3 ray_dir){
    float res = 1.0, t = 0.15;
    for(int i = 0; i < 16; i++) {
        float h = min_distance(ray_pos + ray_dir*t);
        if(h < 0.01) return 0.0;
        res = min(res, 2.0 * h/t);
        t += h*0.9;
    }
    return res;
}

vec3 ray_marching(vec3 ray_pos, vec3 ray_dir, vec3 light_position[NUM_OF_LIGHTS]) {
    vec3 pos = intersection_point(ray_pos, ray_dir);
    vec3 normal = calculate_normal(pos);
    vec3 color = color_of_closest_object(pos);
    vec3 gi = color_lerp(pos);
    vec3 final_color = vec3(0.0);
    //float shininess = 50.0;
    vec3 light_dir1 = normalize(light_position[0] - pos);
    vec3 light_dir2 = normalize(light_position[1] - pos);
    vec3 viewer_dir = normalize(ray_pos - pos);
    vec3 reflection1 = normalize(light_dir1 + viewer_dir);
    vec3 reflection2 = normalize(light_dir2 + viewer_dir);
    float diffuse1 = dot(normal, light_dir1);
    float diffuse2 = dot(normal, light_dir2);
    float specular1 = max(0.0, pow(dot(normal, reflection1), shininess)) * float(diffuse1 > 0.0);
    float specular2 = max(0.0, pow(dot(normal, reflection2), shininess)) * float(diffuse2 > 0.0);
    diffuse1 = 0.5 + 0.5 * diffuse1;
    diffuse2 = 0.5 + 0.5 * diffuse2;
    float ambient_occ = ambient_occlusion(pos, normal);
    float shadow1 = soft_shadow(pos, normalize(light_position[0]));
    float shadow2 = soft_shadow(pos, normalize(light_position[1]));
    final_color += vec3(diffuse1 * diffuse2 * (0.5 + 0.5 * (shadow1 + shadow2)) * ambient_occ * (color + gi) + specular1 + specular2);

    return final_color;
}

/**
TODO:
* try to think of how to add a second sourse of light
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

    color = vec4(ray_marching(ray_pos, ray_dir, light_position), 1.0);
}


