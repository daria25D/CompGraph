#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

const int NUM_OF_LIGHTS = 2;
const float MAX_DIST = 100;

struct Material {
    vec3  color;
    float shininess;
    float k_diffuse;
    float k_specular;
    float k_ambient;
    float k_occlusion;
    float k_reflection;
};
//objects
float sphere; //current distance
vec3  sphere_centre     = vec3(-2.0, 0.0, 0.0);
float sphere_radius     = 1.0;

float cone_for_sphere;
vec3  cone_for_sphere_centre     = vec3(-2.0, -1.0, 0.0);
vec2  cone_for_sphere_dimensions = vec2(0.7, 1.0);

float ellipsoid;
vec3  ellipsoid_centre     = vec3(2.0, -0.95, 0.0);
vec3  ellipsoid_dimensions = vec3(0.5, 0.3, 0.3);

float torus;
vec3  torus_centre     = vec3(2.0, -2.5, 0.0);
vec2  torus_dimensions = vec2(1.2, 0.15);

float plane;
vec3  plane_centre     = vec3(-1.0, -1.0, -1.0);
vec4  plane_normal     = vec4(0.0, 1.0, 0.0, 5.0);

float cube;
vec3  cube_centre     = vec3(2.0, -4.5, 0.0);
vec3  cube_dimensions = vec3(1.7, 1.5, 1.5);

Material material;

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
float distance_from_cone( vec3 p, vec3 c, vec2 r) {
//    float q = length(p.xy - c.xy);
//    return dot(normalize(r), vec2(q, p.z - c.z));
    float height = r.x;
    float radius = r.y;
    vec2 q = vec2(length(p.xz - c.xz), p.y - c.y);
    vec2 tip = q - vec2(0, height);
    vec2 mantleDir = normalize(vec2(height, radius));
    float mantle = dot(tip, mantleDir);
    float d = max(mantle, -q.y);
    float projected = dot(tip, vec2(mantleDir.y, -mantleDir.x));

    // distance to tip
    if ((q.y > height) && (projected < 0)) {
        d = max(d, length(tip));
    }

    // distance to base ring
    if ((q.x > radius) && (projected > length(vec2(height, radius)))) {
        d = max(d, length(q - vec2(radius, 0)));
    }
    return d;
}
//changing the final shape
float intersect(float dist_a, float dist_b) {
    return max(dist_a, dist_b);
}
float smooth_union(float d1, float d2, float k) {
    float h = clamp( 0.5 + 0.5*(d2 - d1) / k, 0.0, 1.0 );
    return mix(d2, d1, h) - k*h * (1.0 - h);
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
        return smooth_union(distance_from_sphere(p, sphere_centre, sphere_radius),
                            distance_from_cone(p, cone_for_sphere_centre, cone_for_sphere_dimensions), 2);
    } else if (num == 1) {
        return distance_from_ellipsoid(p,  ellipsoid_centre, ellipsoid_dimensions);
    } else if (num == 2) {
        return twist_torus(p, torus_centre, torus_dimensions, 0);
    } else if (num == 3) {
        return distance_from_plane(p, plane_centre, normalize(plane_normal));
    } else if (num == 4) {
        return distance_from_cube(p, cube_centre, cube_dimensions);
    }
}
//minimal distance to any surface
float min_distance(vec3 p) {
    sphere = distance_to_surface(p, 0);
    ellipsoid = distance_to_surface(p, 1);
    torus = distance_to_surface(p, 2);
    plane = distance_to_surface(p, 3);
    cube = distance_to_surface(p, 4);

    return min(cube, min(min(sphere, ellipsoid), min(torus, plane)));
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

Material sphere_material() {
    Material sphere_mat;
    sphere_mat.color        = vec3(0.9, 0.1, 0.1);
    sphere_mat.shininess    = 100.0;
    sphere_mat.k_diffuse    = 0.33;
    sphere_mat.k_specular   = 0.9;
    sphere_mat.k_ambient    = 0.33;
    sphere_mat.k_occlusion  = 5.0;
    sphere_mat.k_reflection = 0.0;
    return sphere_mat;
}

Material ellipsoid_material() {
    Material ellipsoid_mat;
    ellipsoid_mat.color        = vec3(0.9, 0.7, 0.8);
    ellipsoid_mat.shininess    = 200.0;
    ellipsoid_mat.k_diffuse    = 0.8;
    ellipsoid_mat.k_specular   = 0.8;
    ellipsoid_mat.k_ambient    = 0.6;
    ellipsoid_mat.k_occlusion  = 4.0;
    ellipsoid_mat.k_reflection = 0.0;
    return ellipsoid_mat;
}

Material torus_material() {
    Material torus_mat;
    torus_mat.color        = vec3(0.9, 0.7, 0.0);
    torus_mat.shininess    = 70.0;
    torus_mat.k_diffuse    = 0.6;
    torus_mat.k_specular   = 0.7;
    torus_mat.k_ambient    = 0.5;
    torus_mat.k_occlusion  = 4.0;
    torus_mat.k_reflection = 0.0;
    return torus_mat;
}
Material plane_material() {
    Material plane_mat;
    plane_mat.color        = vec3(0.8);
    plane_mat.shininess    = 2.0;
    plane_mat.k_diffuse    = 0.33;
    plane_mat.k_specular   = 0.5;
    plane_mat.k_ambient    = 0.33;
    plane_mat.k_occlusion  = 5.0;
    plane_mat.k_reflection = 1.0;
    return plane_mat;
}
Material cube_material() {
    Material cube_mat;
    cube_mat.color        = vec3(0.5, 0.0, 0.5);
    cube_mat.shininess    = 20.0;
    cube_mat.k_diffuse    = 0.33;
    cube_mat.k_specular   = 0.5;
    cube_mat.k_ambient    = 0.33;
    cube_mat.k_occlusion  = 4.0;
    cube_mat.k_reflection = 0.0;
    return cube_mat;
}

vec3 color_of_closest_object(vec3 p) {
    float d = 1e10;
    vec3 color = vec3(0.0);
    if (sphere < d) {
        d = sphere;
        material = sphere_material();
    }
    if (ellipsoid < d) {
        d = ellipsoid;
        material = ellipsoid_material();
    }
    if (torus < d) {
        d = torus;
        material = torus_material();
    }
    if (plane < d) {
        d = plane;
        material = plane_material();
    } if (cube < d) {
        d = cube;
        material = cube_material();
    }
    return material.color;
}
//to prevent self-illumination
vec3 color_lerp(vec3 p) {
    vec3 color = vec3(0.0);

    float sphere_weight = 0.0;
    float ellipsoid_weight = 0.0;
    float torus_weight = 0.0;
    float plane_weight = 0.0;
    float cube_weight = 0.0;

    float eps = 0.01;
    float GI = 0.7;
    if (sphere > eps) sphere_weight = GI/(sphere + 1.0);
    if (ellipsoid > eps) ellipsoid_weight = GI/(ellipsoid + 1.0);
    if (torus > eps) torus_weight = GI/(torus + 1.0);
    if (plane > eps) plane_weight = GI/(plane + 1.0);
    if (cube > eps) cube_weight - GI/(cube + 1.0);

    color = sphere_material().color * sphere_weight +
            ellipsoid_material().color * ellipsoid_weight +
            torus_material().color * torus_weight +
            plane_material().color * plane_weight +
            cube_material().color * cube_weight;
    return color * color * color * 0.15;
}

float intersection_point(vec3 ray_pos, vec3 ray_dir) {
    float d, t = 0.0;
    for (int i = 0; i < 175; i++) {
        d = min_distance(ray_pos + t*ray_dir);
        if (abs(d) < 0.01) return t;//ray_pos + t*ray_dir; //min_dist
        t += d;
        if (abs(d) > MAX_DIST) return t;//ray_pos + t*ray_dir; //max_dist
    }
    return t;//ray_pos + t * ray_dir;
}

float ambient_occlusion(vec3 p, vec3 n, float k){
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
    return clamp(1.0 - k*a, 0.0, 1.0);
}

float soft_shadow(vec3 ray_pos, vec3 ray_dir){
    float res = 1.0, t = 0.15;
    for(int i = 0; i < 16; i++) {
        float h = min_distance(ray_pos + ray_dir*t);
        if(h < 0.01) return 0.0;
        res = min(res, 8.0 * h/t);
        t += h*0.9;
    }
    return clamp(res, 0.0, 1.0);
}


vec3 render(vec3 ray_pos, vec3 ray_dir, vec3 light_position[NUM_OF_LIGHTS]) {
    float t = intersection_point(ray_pos, ray_dir);
    vec3 pos = ray_pos + t*ray_dir;
    vec3 normal = calculate_normal(pos);
    vec3 color = color_of_closest_object(pos);
    vec3 gi = color_lerp(pos);
    if (t > MAX_DIST) {color = vec3(0.8); gi = vec3(0.0);}
    vec3 final_color = vec3(0.0);
    vec3 light_dir1 = normalize(light_position[0] - pos);
    vec3 light_dir2 = normalize(light_position[1] - pos);
    vec3 viewer_dir = normalize(ray_pos - pos);
    vec3 reflection1 = normalize(light_dir1 + viewer_dir);
    vec3 reflection2 = normalize(light_dir2 + viewer_dir);
    float diffuse1 = clamp(dot(normal, light_dir1), 0.0, 1.0);
    float diffuse2 = clamp(dot(normal, light_dir2), 0.0, 1.0);
    float specular1 = max(0.0, pow(dot(normal, reflection1), material.shininess)) * float(diffuse1 > 0.0);
    float specular2 = max(0.0, pow(dot(normal, reflection2), material.shininess)) * float(diffuse2 > 0.0);
    //diffuse1 = 0.5 + 0.5 * diffuse1;
    //diffuse2 = 0.5 + 0.5 * diffuse2;
    float ambient_occ = ambient_occlusion(pos, normal, material.k_occlusion);
    float shadow1 = soft_shadow(pos, normalize(light_position[0]));
    float shadow2 = soft_shadow(pos, normalize(light_position[1]));
    vec3 reflect = vec3(0.0);
    final_color += vec3(((material.k_diffuse*(diffuse1 + diffuse2) + material.k_ambient*ambient_occ + material.k_reflection*reflect) * (color + gi))
                        * 0.5*(shadow1 + shadow2) + material.k_specular*(specular1 + specular2)) ;
    final_color *= exp(-0.00005*t*t);
    return final_color;
}
/*
TODO:
* do materials structures instead of assigning everything to different values
*/
void main() {
    vec2 tmp = 0.5 + vec2(fragmentTexCoord.x * g_screenWidth /2, fragmentTexCoord.y * g_screenHeight /2); //try to change
    vec3 ray_dir = normalize(vec3(tmp, -(g_screenWidth)/tan(3.14159265/3)));
    mat3 rdMat = mat3(g_rayMatrix);//multiply rayMatrix 3x3 x rd
    ray_dir = rdMat * ray_dir;
    vec3 ray_pos = vec3(0.0 + g_rayMatrix[3][0], 0.0 + g_rayMatrix[3][1], 5.0 + g_rayMatrix[3][2]); //+ rayMatrix[3][i]

    vec3 light_position[NUM_OF_LIGHTS];
    light_position[0] = vec3(4.0, 7.0, 4.0);
    light_position[1] = vec3(1.0, 7.0, -3.0);


    color = vec4(render(ray_pos, ray_dir, light_position), 1.0);
}


