#version 410
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in ivec2 indices;

uniform mat4 screen_from_world = mat4(1.0);

uniform sampler2D heights;
uniform ivec2 tex_size;
uniform vec3 world_origin;
uniform vec3 world_dimensions;

//out Vertex
//{
//    vec3 world_position;
//    vec3 world_normal;
//    vec2 tex_coords;
//} vertex;

out Vertex
{
    ivec2 indices;
    vec3 world_position;
} vertex;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main(void)
{
    vertex.indices = indices;
//    vec2 tex_coords = (indices + 0.5) / tex_size;
//
//    float height = texture(orb, tex_coords).r;
//
//    vertex.world_position = vec3(indices.x / max(1.0, tex_size.x - 1.0), height, indices.y / max(1.0, tex_size.y - 1.0));
//    vertex.world_position = vertex.world_position * world_dimensions + world_origin;

//    vertex.world_normal = vec3(0, 1, 0);
//    vertex.tex_coords = tex_coords;

//    gl_Position = screen_from_world * vec4(vertex.world_position, 1.0);
    gl_Position = vec4(0, 0, 0, 1);
}
