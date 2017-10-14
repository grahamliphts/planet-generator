#version 400

out vec4 FragColor;
in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
in float gPrimitive;
uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;
uniform mat4 Projection;
uniform mat4 Model;
uniform mat4 View;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main()
{
    vec3 N = normalize(gFacetNormal);
    vec3 L = LightPosition;
    //vec4 LightLocation = Projection * View * Model * vec4(LightPosition,1);
    //vec3 L = LightLocation.xyz;
    float df = dot(N, L);
    vec3 color = AmbientMaterial + df * DiffuseMaterial;

    //float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    //float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
    //color = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * color;

    FragColor = vec4(color, 1.0);
}