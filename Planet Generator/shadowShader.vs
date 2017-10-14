// Used for shadow lookup
varying vec4 ShadowCoord;
uniform vec4 color = vec4(0.4,0.4,1,0);
in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{


     	ShadowCoord= gl_TextureMatrix[7] * gl_Vertex;
  
		//gl_Position = ftransform();
		gl_Position = projection * view * model * vec4(position, 1.0f);

		gl_FrontColor = color;
}

