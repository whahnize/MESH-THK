
-- Vertex

attribute vec4 Position;
attribute vec3 Normal;

varying vec3 vNormal;
varying vec3 vPosition;
varying float vRadius;

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;
uniform vec2 Size;
void main()
{
    //vPosition = (Modelview * Position).xyz;
    //vNormal = NormalMatrix * Normal;
    //gl_Position = Projection * Modelview * Position;
	gl_Position.xy = Position.xy;
	gl_Position.zw = vec2(0.0,1.0);
	vRadius = Position.z;
}

-- Fragment.Depth

uniform float DepthScale;
uniform float RadiusScale; 
uniform vec2 Size;
varying vec3 vNormal;
varying vec3 vPosition;
varying float vRadius;

void main()
{
    //vec3 N = normalize(vNormal);
    //vec3 P = vPosition;
    //vec3 I = normalize(P);
    //float cosTheta = abs(dot(I, N));
    //float fresnel = pow(1.0 - cosTheta, 4.0);
    //float depth = DepthScale * gl_FragCoord.z;
	//gl_FragColor = vec4(depth, fresnel, 0, 0);

	float r = RadiusScale*vRadius;
    gl_FragColor = vec4(r, r, r, 0);
}

-- Vertex.Quad

attribute vec4 Position;

void main()
{
    gl_Position = Position;
}

-- Fragment.Absorption

uniform sampler2D Sampler;
uniform vec2 Size;
uniform vec3 DiffuseMaterial;

void main()
{
    vec2 texCoord = gl_FragCoord.xy / Size;
    float thickness = abs(texture2D(Sampler, texCoord).r);
    if (thickness <= 0.0)
    {
        discard;
    }
    float sigma = 8.0;
	float intensity =  thickness; 
    //float intensity = log(sigma * thickness);
	//float fresnel = 1.0 - texture2D(Sampler, texCoord).g;
	//float intensity = fresnel * exp(-sigma * thickness);
    //gl_FragColor = vec4(fresnel * DiffuseMaterial, 1);
	gl_FragColor = vec4(intensity,0,0, 0);
}
