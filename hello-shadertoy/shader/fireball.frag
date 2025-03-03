
#version 450

layout(binding = 0) uniform UniformBufferObject {
    vec4 iMouse;
    float iTime;
    vec3 iResolution;
} ubo;

layout(location = 0) out vec4 fragColor;

#define PI 3.14159265359

// Let's draw something that is not a solid color.
void main()
{
	// choose two colors
	vec3 color1 = vec3(0.886, 0.576, 0.898);
	vec3 color2 = vec3(0.537, 0.741, 0.408);
	vec3 pixel;
	
	// if the x coordinate is greater than 100 then plot color1
	// else plot color2
	float widthOfStrip = 100.0;
	if( gl_FragCoord.x > widthOfStrip ) {
		pixel = color2;
	} else {
		pixel = color1;
	}
	
	fragColor = vec4(pixel, 1.0);
}

