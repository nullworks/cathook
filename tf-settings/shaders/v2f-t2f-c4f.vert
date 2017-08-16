// 2D drawing shader - modified freetype-gl shader
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec2 vertex;
attribute vec2 tex_coord;
attribute vec4 color;

void main()
{
    gl_TexCoord[0].xy = tex_coord.xy;
    gl_FrontColor     = color;
    gl_Position       = projection*(view*(model*vec4(vertex,0.0,1.0)));
}
