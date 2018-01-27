--Vertex.Normal

attribute vec4 Position;
attribute vec3 Normal;

varying vec3 vNormal;
varying vec3 vPosition;

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;

void main()
{
	vPosition = (Modelview * Position).xyz;
	vNormal = NormalMatrix * Normal;
	gl_Position = Projection * Modelview * Position;
}


--Fragment.Normal

uniform float DepthScale;
varying vec3 vNormal;
varying vec3 vPosition;
void main()
{
	//vec3 N = normalize(vNormal);
	vec3 P = vPosition;
	vec3 I = normalize(P);
	//float cosTheta = abs(dot(I, N));
	//float fresnel = pow(1.0 - cosTheta, 4.0);
	float depth = DepthScale * gl_FragCoord.z;

	//gl_FragColor = vec4(depth, fresnel, 0, 0);
	gl_FragColor = vec4(depth, 0, 0, 0);
}

-- Vertex.Z

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

-- Fragment.Z

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

	float r = DepthScale*vRadius;
    gl_FragColor = vec4(r, r, r, 0);
}