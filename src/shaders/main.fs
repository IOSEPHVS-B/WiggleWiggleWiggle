#version 120

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// NOTE: Add here your custom variables
uniform vec3 palette[4];

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    
    // NOTE: Implement here your fragment shader code
    vec4 finalColor = texelColor * colDiffuse * fragColor;

    vec3 palCol = palette[ int(finalColor.r * 3.99) ];

    gl_FragColor = vec4(palCol.r, palCol.g, palCol.b, finalColor.a);
}