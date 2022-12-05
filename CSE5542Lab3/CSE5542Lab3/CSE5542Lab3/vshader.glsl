/***************************
 * File: vshader.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

#version 150 //  Comment/un-comment this line to resolve compilation errors
//      due to different settings of the default GLSL version

in  vec3 vPosition;
in  vec3 vColor;
out vec4 color;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
    vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, 1.0); 

    // JC: build-in variable in GLSL
    //  gl_Position is the first one we see here.
    //  The OpenGL shading language defines a number of special variables for the various shader stages.
    //  They are usually for communicating with certain fixed-functionality. 
    //  By convention, all predefined variables start with "gl_"; 
    //     no user-defined variables may start with this.
    //  see here: https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL) for the list of build in variables.
    gl_Position = projection * view * vPosition4;

    color = vColor4;
} 
