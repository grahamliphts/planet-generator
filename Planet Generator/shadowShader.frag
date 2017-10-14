uniform sampler2D ShadowMap;

varying vec4 ShadowCoord;
uniform vec4 color = vec4(0.1,0.1,1,0);
void main()
{	
	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w ;
	
	// Used to lower moiré pattern and self-shadowing
	shadowCoordinateWdivide.z += 0.0005;
	
	
	float distanceFromLight = texture2D(ShadowMap,shadowCoordinateWdivide.st).z;
	
	
 	float shadow = 1.0;
 	if (ShadowCoord.w > 0.0)
 		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
  	
	
  	
	//gl_FragColor =	 shadow * color;
	gl_FragColor = color;
  
}

