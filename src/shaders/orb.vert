#version 410
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 tex_coords;

uniform mat4 screen_from_world        = mat4(1.0);
uniform mat4 world_from_local         = mat4(1.0);
uniform mat3 world_from_local_normals = mat3(1.0);
uniform sampler2D tex;

out Vertex
{
    vec3 world_position;
    vec3 world_normal;
    vec2 tex_coords;
} vertex;

out gl_PerVertex
{
  vec4 gl_Position;
};

const float PI = 3.1415926536;

const int degree = 2;
const int num_points = 128;
const int order = degree + 1;
const int num_spans = num_points - degree;

vec3 rotate_vector(in vec3 v, in float angle, in vec3 axis)
{
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    mat3 rot = mat3(
        oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
        oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s,
        oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
    );

    return rot * v;
}

vec3 sphere_point(in float phi, in float theta)
{
    return vec3(cos(theta) * sin(phi),
                cos(phi),
                sin(theta) * sin(phi));
}

float U(in int i)
{
    return clamp(float(i - degree), 0.0, float(num_spans));
}

mat3 ders_basis_funs(in int i, in float u)
{
    mat3 ders;
    mat3 ndu;
    float left[order];
    float right[order];

    ndu[0][0] = 1.0;

    for (int j = 1; j <= degree; ++j)
    {
        left[j] = u - U(i + 1 - j);
        right[j] = U(i + j) - u;
        float saved = 0.0;

        for (int r = 0; r < j; ++r)
        {
            // lower triangle
            ndu[j][r] = right[r + 1] + left[j - r];
            float tmp = ndu[r][j - 1] / ndu[j][r];

            // upper triangle
            ndu[r][j] = saved + right[r + 1] * tmp;
            saved = left[j - r] * tmp;
        }
        ndu[j][j] = saved;
    }

    // load the basis functions
    for (int j = 0; j <= degree; ++j)
    {
        ders[0][j] = ndu[j][degree];
    }

    for (int r = 0; r <= degree; ++r)
    {
        int s1 = 0;
        int s2 = 1;

        mat3 a;
        a[0][0] = 1.0;

        {
            float d = 0.0;
            int rk = r - 1;
            int pk = degree - 1;
            if (r >= 1)
            {
                a[s2][0] = a[s1][0] / ndu[pk + 1][max(rk, 0)];
                d = a[s2][0] * ndu[rk][pk];
            }
            int j1 = (rk >= -1 ? 1 : -rk);
            int j2 = (r - 1 <= pk ? 0 : degree - r);

            for (int j = j1; j <= j2; ++j)
            {
                a[s2][j] = (a[s1][j] - a[s1][j - 1]) / ndu[pk + 1][rk + j];
                d += a[s2][j] * ndu[rk + j][pk];
            }
            if (r <= pk)
            {
                a[s2][1] = -a[s1][0] / ndu[pk + 1][r];
                d += a[s2][1] * ndu[r][pk];
            }
            ders[1][r] = d;
        }
    }

    return ders;
}

void curve_point(in float t, in float theta, out vec3 point, out vec3 normal)
{
    if (t == 1.0)
    {
        normal = vec3(0, -1, 0);
        point = normal * (0.5 + texture(tex, vec2(1.0, 0.5f)).r * 0.5);
        return;
    }
    float u = t * num_spans;
    float u_floor = floor(u);

    // compute normalized points on sphere
    float phi_scale = PI / (num_points - 1);
    vec3 P1 = sphere_point((u_floor + 0.0) * phi_scale, theta);
    vec3 P2 = sphere_point((u_floor + 1.0) * phi_scale, theta);
    vec3 P3 = sphere_point((u_floor + 2.0) * phi_scale, theta);

    // scale points by fft values
    float tex_scale = 1.0 / num_points;
    P1 *= 0.5 + texture(tex, vec2((u_floor + 0.5) * tex_scale, 0.5f)).r * 0.5;
    P2 *= 0.5 + texture(tex, vec2((u_floor + 1.5) * tex_scale, 0.5f)).r * 0.5;
    P3 *= 0.5 + texture(tex, vec2((u_floor + 2.5) * tex_scale, 0.5f)).r * 0.5;

    int span = int(u_floor) + degree;
    mat3 N = ders_basis_funs(span, u);

    point = N[0][0] * P1 + N[0][1] * P2 + N[0][2] * P3;
    vec3 deriv = N[1][0] * P1 + N[1][1] * P2 + N[1][2] * P3;

    normal = normalize(rotate_vector(deriv, -PI * 0.5, vec3(-sin(theta), 0, cos(theta))));
    return;
}

void main(void)
{
    float theta = tex_coords.x * 2.0 * PI;

    vec3 point, normal;
    curve_point(tex_coords.y, theta, point, normal);

    vertex.world_position = vec3(world_from_local * vec4(point, 1.0));
    vertex.world_normal = normalize(world_from_local_normals * normal);
    vertex.tex_coords = vec2(tex_coords);

    gl_Position = screen_from_world * vec4(vertex.world_position, 1.0);
}
