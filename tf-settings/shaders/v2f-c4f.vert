// 2D drawing shader
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec2 vertex;
attribute vec4 color;

void main()
{
    gl_FrontColor = color;
    gl_Position = projection*(view*(model*vec4(vertex,0.0,1.0)));
}
