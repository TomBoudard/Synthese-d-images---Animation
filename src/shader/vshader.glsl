#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform bool isMesh;

attribute vec3 a_position;
attribute vec3 a_color;
// attribute vec2 a_texcoord;
attribute float a_weight0;
attribute float a_weight1;
attribute float a_weight2;
attribute float a_weight3;
attribute vec3 a_mat0;
attribute vec3 a_mat1;
attribute vec3 a_mat2;
attribute vec3 a_mat3;

// varying vec2 v_texcoord;
varying vec3 v_color;

//! [0]
void main()
{
    // Calculate vertex position in screen space
    if (isMesh){
        gl_Position = mvp_matrix * vec4(a_weight0*(a_mat0+a_position) + a_weight1*(a_mat1+a_position) + a_weight2*(a_mat2+a_position) + a_weight3*(a_mat3+a_position), 1.);
        // gl_Position = mvp_matrix * vec4(a_position, 1.);
        v_color = a_color;
    }
    else{
        gl_Position = mvp_matrix * vec4(a_position, 1.);
        v_color = a_color;
    }

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    // v_texcoord = a_texcoord;

}
//! [0]
