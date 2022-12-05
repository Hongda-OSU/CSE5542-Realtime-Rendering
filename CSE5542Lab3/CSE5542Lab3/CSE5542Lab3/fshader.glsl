/*****************************
 * File: fshader.glsl
 *       A simple fragment shader
 *****************************/

#version 150  
//#version 330  core //  Comment/un-comment this line to resolve compilation errors
              //      due to different settings of the default GLSL version
              

in  vec4 color;
out vec4 fColor;

void main() 
{ 
    // JC: this line says just output whatever color you have throw "in" to the 
    // pipeline. 
    fColor = color;

    // JC: comment out the line above and try this one to see the effect. 
    //  this says hey, I just want everything yellow. So output yellow.
    //fColor = vec4(1, 1, 0, 0);
    //fColor = vec4(0, 0, 1, 0);
} 

