#version 330
uniform mat4 g_rayMatrix;
//uniform vec3 cam_pos;
uniform int g_screenWidth;
uniform int g_screenHeight;

in vec2 fragmentTexCoord;

out vec4 color;

const int NUM_OF_LIGHTS = 2;
const float MAX_DIST = 100;
//objects
float sphere; //current distance
vec3  sphere_centre     = vec3(0.0, 0.0, 0.0);
float sphere_radius     = 1.0;
vec3  sphere_color      = vec3(0.9, 0.1, 0.1);
float sphere_shininess  = 100.0;
float sphere_diffuse    = 0.33;
float sphere_specular   = 0.4;
float sphere_ambient    = 0.33;
float sphere_occlusion  = 5.0;
float sphere_reflection = 0.2;


float ellipsoid;
vec3  ellipsoid_centre     = vec3(2.0, 0.0, -2.0);
vec3  ellipsoid_dimensions = vec3(1.0, 0.5, 2.0);
vec3  ellipsoid_color      = vec3(0.0, 0.7, 0.0);
float ellipsoid_shininess  = 2.0;
float ellipsoid_diffuse    = 0.4;
float ellipsoid_specular   = 0.2;
float ellipsoid_ambient    = 0.4;
float ellipsoid_occlusion  = 4.0;
float ellipsoid_reflection = 0.0;

float torus;
vec3  torus_centre     = vec3(-1.0, -3.0, 0.0);
vec2  torus_dimensions = vec2(1.0, 0.5);
vec3  torus_color      = vec3(0.2, 0.7, 0.7);
float torus_shininess  = 50.0;
float torus_diffuse    = 0.33;
float torus_specular   = 0.33;
float torus_ambient    = 0.33;
float torus_occlusion  = 4.0;
float torus_reflection = 1.0;

float plane;
vec3  plane_centre     = vec3(-1.0, -1.0, -1.0);
vec4  plane_normal     = vec4(0.0, 1.0, 0.0, 5.0);
vec3  plane_color      = vec3(0.8);
float plane_shininess  = 1.0;
float plane_diffuse    = 0.33;
float plane_specular   = 0.2;
float plane_ambient    = 0.33;
float plane_occlusion  = 5.0;
float plane_reflection = 0.1;

float cube_for_sphere;
vec3  cube_for_sphere_centre     = vec3(0.0, 0.0, 0.0);
vec3  cube_for_sphere_dimensions = vec3(1.0);

float shininess;
float k_diffuse;
float k_specular;
float k_ambient;
float k_occlusion;
float k_reflection;

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
    ellipsoid = distance_to_surface(p, 1);
    torus = distance_to_surface(p, 2);
    plane = distance_to_surface(p, 3);

    return min(min(sphere, ellipsoid), min(torus, plane));
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
        k_diffuse = sphere_diffuse;
        k_specular = sphere_specular;
        k_ambient = sphere_ambient;
        k_occlusion = sphere_occlusion;
        k_reflection = sphere_reflection;
    }
    if (ellipsoid < d) {
        d = ellipsoid;
        color = ellipsoid_color;
        shininess = ellipsoid_shininess;
        k_diffuse = ellipsoid_diffuse;
        k_specular = ellipsoid_specular;
        k_ambient = ellipsoid_ambient;
        k_occlusion = ellipsoid_occlusion;
        k_reflection = ellipsoid_reflection;
    }
    if (torus < d) {
        d = torus;
        color = torus_color;
        shininess = torus_shininess;
        k_diffuse = torus_diffuse;
        k_ambient = torus_ambient;
        k_specular = torus_ambient;
        k_occlusion = torus_occlusion;
        k_reflection = torus_reflection;
    }
    if (plane < d) {
        d = plane;
        color = plane_color;
        shininess = plane_shininess;
        k_diffuse = plane_diffuse;
        k_ambient = plane_ambient;
        k_specular = plane_ambient;
        k_occlusion = plane_occlusion;
        k_reflection = plane_reflection;
    }
    return color;
}
//to prevent self-illumination
vec3 color_lerp(vec3 p) {
    vec3 color = vec3(0.0);

    float sphere_weight = 0.0;
    float ellipsoid_weight = 0.0;
    float torus_weight = 0.0;
    float plane_weight = 0.0;

    float eps = 0.01;
    float GI = 0.7;
    if (sphere > eps) sphere_weight = GI/(sphere + 1.0);
    if (ellipsoid > eps) ellipsoid_weight = GI/(ellipsoid + 1.0);
    if (torus > eps) torus_weight = GI/(torus + 1.0);
    if (plane > eps) plane_weight = GI/(plane + 1.0);

    color = sphere_color * sphere_weight +
            ellipsoid_color * ellipsoid_weight +
            torus_color * torus_weight +
            plane_color * plane_weight;
    return color * color * color * 0.15;
}

float intersection_point(vec3 ray_pos, vec3 ray_dir) {
    float d, t = 0.0;
    for (int i = 0; i < 175; i++) {
        d = min_distance(ray_pos + t*ray_dir);
        if (abs(d) < 0.01) return t;//ray_pos + t*ray_dir; //min_dist
        t += d;
        if (abs(d) > 120) return t;//ray_pos + t*ray_dir; //max_dist
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

vec3 reflection(vec3 rro, vec3 ray_dir, vec3 normal, vec3 ray_pos, vec3 light_position)
{
    vec3 res = vec3(0.80, 0.75, 0.70)*0.35; //ambient reflection
    int chk = 1;
    for(int j = 0; j<3; j++)		// 3 reflections
    {
		ray_dir = reflect(ray_dir, normal);
		float tmax = 50.0;
   		float t = 0.1;
    	vec3 pos = rro;
    	float d = 0.0;
    	if(chk == 1)
    	{
			for(int i = 0; i<256; i++ )
			{
        		if(t>=tmax) break;
				pos = rro+ray_dir*t;
				d = min_distance(pos);
				if(d < 0.001) break;
				t += d*(0.35+t/tmax);
			}
    	}
    	chk = 0;
    	if(d < 0.001)
		{
    		float ks = 0.1;
			float kd = 0.1;
			float ka = 0.04;
        	float a = 3.0;
			float aof = 0.06;
			float ss = 0.2;
            vec3 color = color_of_closest_object(pos);
			vec3 l = normalize(light_position-pos);
			normal = calculate_normal(pos);
			vec3 v = normalize(ray_pos - pos);
        	vec3 h = normalize(l+v);

			float illumination  = ka*ambient_occlusion(pos, normal, aof)
								+ kd*max(dot(l,normal),0.0)
                         	    + ks*pow(max(dot(normal,h),0.0),a)
								+ ss*soft_shadow(pos, light_position);
        	illumination = max(illumination, 0.0);
			res += color*illumination*(1.0/pow(2.0,float(j)));
        	chk = 1;
        	rro = pos;
		}
    }
	return res;
}


vec3 render(vec3 ray_pos, vec3 ray_dir, vec3 light_position[NUM_OF_LIGHTS]) {
    float t = intersection_point(ray_pos, ray_dir);
    vec3 pos = ray_pos + t*ray_dir;
    /*vec3 final_color = phong(ray_pos, ray_dir, pos, light_position[0]);
    final_color += phong(ray_pos, ray_dir, pos, light_position[1]);*/
    //float shadow = 0.5*soft_shadow(ray_pos, light_position[0]) + 0.5*soft_shadow(ray_pos, light_position[1]);
    //final_color *= shadow;
    vec3 normal = calculate_normal(pos);
    vec3 color = color_of_closest_object(pos);
    vec3 gi = color_lerp(pos);
    if (t > MAX_DIST) color = vec3(0.8, 0.8, 0.8);
    vec3 final_color = vec3(0.0);
    vec3 light_dir1 = normalize(light_position[0] - pos);
    vec3 light_dir2 = normalize(light_position[1] - pos);
    vec3 viewer_dir = normalize(ray_pos - pos);
    vec3 reflection1 = normalize(light_dir1 + viewer_dir);
    vec3 reflection2 = normalize(light_dir2 + viewer_dir);
    float diffuse1 = clamp(dot(normal, light_dir1), 0.0, 1.0);
    float diffuse2 = clamp(dot(normal, light_dir2), 0.0, 1.0);
    float specular1 = max(0.0, pow(dot(normal, reflection1), shininess)) * float(diffuse1 > 0.0);
    float specular2 = max(0.0, pow(dot(normal, reflection2), shininess)) * float(diffuse2 > 0.0);
    //diffuse1 = 0.5 + 0.5 * diffuse1;
    //diffuse2 = 0.5 + 0.5 * diffuse2;
    if (shininess > 100) {
        specular1 = specular1/5;
        specular2 = specular2/5;
    }
    float ambient_occ = ambient_occlusion(pos, normal, k_occlusion);
    float shadow1 = soft_shadow(pos, normalize(light_position[0]));
    float shadow2 = soft_shadow(pos, normalize(light_position[1]));
    final_color += vec3(((k_diffuse*(diffuse1 + diffuse2) + k_ambient*ambient_occ) * (color + gi)) * 0.5*(shadow1 + shadow2) + k_specular*(specular1 + specular2)) ;
    final_color *= exp(-0.0005*t*t);
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
    light_position[0] = vec3(4.0, 7.0, 0.0);
    light_position[1] = vec3(0.0, 7.0, 3.0);

    color = vec4(render(ray_pos, ray_dir, light_position), 1.0);
}


