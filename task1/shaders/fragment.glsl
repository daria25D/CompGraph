#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

const int NUM_OF_LIGHTS = 2;
const float MAX_DIST = 50;
const float REFRACTION_OUTSIDE = 1.0002;
const float REFLECTIVITY = 0.01;

bool count_refract = true;

struct Material {
    vec3  color;
    float shininess;
    float k_diffuse;
    float k_specular;
    float k_ambient;
    float k_occlusion;
    float k_refraction;
};
//objects
int   SPHERE=1; //current distance
vec3  sphere_centre     = vec3(-2.0, -4.8, 2.0);
float sphere_radius     = 0.9;

vec3   octahedron_for_sphere_centre    = vec3(-2.0, -4.8, 2.0);
float  octahedron_for_sphere_dimension = 1.2;

int   ELLIPSOID=2;
vec3  ellipsoid_centre     = vec3(2.0, -0.95, 0.0);
vec3  ellipsoid_dimensions = vec3(0.5, 0.3, 0.3);

int   TORUS=3;
vec3  torus_centre     = vec3(2.0, -2.5, 0.0);
vec2  torus_dimensions = vec2(1.2, 0.15);

int   PLANE=4;
vec3  plane_centre     = vec3(-1.0, -1.0, -1.0);
vec4  plane_normal     = vec4(0.0, 1.0, 0.0, 5.0);

int   CUBE=5;
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
float distance_from_octahedron(vec3 p, float s) {
    p = abs(p);
    float m = p.x+p.y+p.z-s;
    vec3 q;
         if(3.0*p.x < m) q = p.xyz;
    else if(3.0*p.y < m) q = p.yzx;
    else if(3.0*p.z < m) q = p.zxy;
    else return m * 0.57735027;

    float k = clamp(0.5*(q.z - q.y + s), 0.0, s);
    return length(vec3(q.x, q.y - s + k, q.z - k));
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

//minimal distance to any surface
vec2 min_distance(vec3 p) {
    vec2 res = vec2(MAX_DIST*20, 0);

    float dist = distance_from_ellipsoid(p,  ellipsoid_centre, ellipsoid_dimensions);
    if (res.x > dist) res = vec2(dist, ELLIPSOID);
    dist = twist_torus(p, torus_centre, torus_dimensions, 0);
    if (res.x > dist) res = vec2(dist, TORUS);
    dist = distance_from_plane(p, plane_centre, normalize(plane_normal));
    if (res.x > dist) res = vec2(dist, PLANE);
    dist = distance_from_cube(p, cube_centre, cube_dimensions);
    if (res.x > dist) res = vec2(dist, CUBE);
    if (count_refract) dist = smooth_union(distance_from_sphere(p, sphere_centre, sphere_radius),
                                 distance_from_octahedron(p - octahedron_for_sphere_centre, octahedron_for_sphere_dimension), 0.5);
    if (res.x > dist) res = vec2(dist, SPHERE);

    return res;
}
//normal for a given point
vec3 calculate_normal(vec3 p) {
    vec3 small_step = vec3(0.001, 0.0, 0.0);
    vec3 normal = vec3(0.0);
    //gradients calculation
    normal.x = min_distance(p + small_step.xyy).x - min_distance(p - small_step.xyy).x;
    normal.y = min_distance(p + small_step.yxy).x - min_distance(p - small_step.yxy).x;
    normal.z = min_distance(p + small_step.yyx).x - min_distance(p - small_step.yyx).x;

    return normalize(normal);
}

Material sphere_material() {
    Material sphere_mat;
    sphere_mat.color        = vec3(0.9, 0.3, 0.3);
    sphere_mat.shininess    = 100.0;
    sphere_mat.k_diffuse    = 0.33;
    sphere_mat.k_specular   = 0.9;
    sphere_mat.k_ambient    = 0.33;
    sphere_mat.k_occlusion  = 5.0;
    sphere_mat.k_refraction = 1.2;
    return sphere_mat;
}

Material ellipsoid_material() {
    Material ellipsoid_mat;
    ellipsoid_mat.color        = vec3(0.9, 0.7, 0.8);
    ellipsoid_mat.shininess    = 100.0;
    ellipsoid_mat.k_diffuse    = 0.8;
    ellipsoid_mat.k_specular   = 0.8;
    ellipsoid_mat.k_ambient    = 0.6;
    ellipsoid_mat.k_occlusion  = 4.0;
    ellipsoid_mat.k_refraction = 0.0;
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
    torus_mat.k_refraction = 0.0;
    return torus_mat;
}
Material plane_material() {
    Material plane_mat;
    plane_mat.color        = vec3(0.8);
    plane_mat.shininess    = 5.0;
    plane_mat.k_diffuse    = 0.33;
    plane_mat.k_specular   = 0.5;
    plane_mat.k_ambient    = 0.33;
    plane_mat.k_occlusion  = 5.0;
    plane_mat.k_refraction = 0.0;
    return plane_mat;
}
Material cube_material() {
    Material cube_mat;
    cube_mat.color        = vec3(0.5, 0.0, 0.5);
    cube_mat.shininess    = 10.0;
    cube_mat.k_diffuse    = 0.33;
    cube_mat.k_specular   = 0.5;
    cube_mat.k_ambient    = 0.33;
    cube_mat.k_occlusion  = 4.0;
    cube_mat.k_refraction = 0.0;
    return cube_mat;
}

vec3 color_of_closest_object(vec2 obj) {
    vec3 color = vec3(0.0);
    if (obj.y == SPHERE) {
        material = sphere_material();
    }
    if (obj.y == ELLIPSOID) {
        material = ellipsoid_material();
    }
    if (obj.y == TORUS) {
        material = torus_material();
    }
    if (obj.y == PLANE) {
        material = plane_material();
    } if (obj.y == CUBE) {
        material = cube_material();
    }
    return material.color;
}

vec2 intersection_point(vec3 ray_pos, vec3 ray_dir) {
    float t = 0.0;
    vec3 obj = vec3(0.0);
    for (int i = 0; i < 128; i++) {
        obj = vec3(min_distance(ray_pos + t*ray_dir), t);
        if (abs(obj.x) < 0.01) return obj.zy;//min_dist
        t += obj.x;
        if (abs(obj.x) > MAX_DIST) return obj.zy;//max_dist
    }
    return obj.zy;
}

float ambient_occlusion(vec3 p, vec3 n, float k){
    float delta = 0.5;
    float a = 0.0;
    float weight = 0.75;
    float m;
    for(int i = 1; i <= 10; i++) {
        float d = (float(i) / float(10)) * delta;
        a += weight*(d - min_distance(p + n*d).x);
        weight *= 0.5;
    }
    return clamp(1.0 - k*a, 0.0, 1.0);
}

float soft_shadow(vec3 ray_pos, vec3 ray_dir){
    float res = 1.0, t = 0.15;
    for(int i = 0; i < 16; i++) {
        float h = min_distance(ray_pos + ray_dir*t).x;
        if(h < 0.01) return 0.0;
        res = min(res, 8.0 * h/t);
        t += h*0.9;
    }
    return clamp(res, 0.0, 1.0);
}

vec3 glass_refraction(vec3 pos, vec3 ray_dir, vec3 normal, float k_refract, vec3 main_color) {
    vec3 refl = reflect(ray_dir, normal);
    vec2 refl_obj = intersection_point(pos, refl);

    vec3 refl_pos = pos + refl_obj.x*refl;
    vec3 refl_color = color_of_closest_object(refl_obj);
    vec3 refl_normal = calculate_normal(refl_pos);
    float refl_occ = ambient_occlusion(refl_pos, refl_normal, material.k_occlusion);
    refl_color *= refl_occ;
    count_refract = false;
    vec3 refr = refract(ray_dir, normal, 1.0 / k_refract);
    vec2 robj = intersection_point(pos, refr);
    vec3 r_pos = pos + robj.x*refr;
    vec3 r_normal = calculate_normal(r_pos);
    count_refract = true;
    vec3 color = color_of_closest_object(robj);
    float occ = ambient_occlusion(r_pos, r_normal, material.k_occlusion);
    color *= main_color * occ;
    return color;
}

vec3 render(vec3 ray_pos, vec3 ray_dir, vec3 light_position[NUM_OF_LIGHTS]) {
    vec2 obj = intersection_point(ray_pos, ray_dir);
    vec3 pos = ray_pos + obj.x*ray_dir;
    vec3 normal = calculate_normal(pos);

    vec3 color = color_of_closest_object(obj);
    if (obj.x > MAX_DIST) color = vec3(0.8);
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

    float ambient_occ = ambient_occlusion(pos, normal, material.k_occlusion);

    float shadow1 = soft_shadow(pos, normalize(light_position[0]));
    float shadow2 = soft_shadow(pos, normalize(light_position[1]));

    vec3 internal_color = vec3(0.0);
    if (material.k_refraction > 0.0) {
        //refracted = 1.0 - refraction(-viewer_dir, normal, material.k_refraction, REFRACTION_OUTSIDE);
        //vec3 refract_dir = refract(ray_dir, normal, material.k_refraction / REFRACTION_OUTSIDE);
        //internal_color = internal_ray_color(ray_pos + refract_dir*0.001, refract_dir) * refracted;
        final_color += vec3(((material.k_diffuse*(diffuse1 + diffuse2) + material.k_ambient*ambient_occ) * color)
                             *(shadow1 + shadow2) + material.k_specular*(specular1 + specular2)) ;
        final_color = glass_refraction(pos, ray_dir, normal, material.k_refraction, final_color);
    } else {
        final_color += vec3(((material.k_diffuse*(diffuse1 + diffuse2) + material.k_ambient*ambient_occ) * color)
                              * 0.5*(shadow1 + shadow2) + material.k_specular*(specular1 + specular2)) ;
        final_color *= exp(-0.00005*obj.x*obj.x);
    }
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
    light_position[0] = vec3(4.0, 7.0, 7.0);
    light_position[1] = vec3(2.0, 7.0, -3.0);


    color = vec4(render(ray_pos, ray_dir, light_position), 1.0);
}


