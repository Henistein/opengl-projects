#version 410 core

in float Height;
in float Biome;

out vec4 FragColor;

void main()
{
    /*
    vec4 color;
    if (Biome == 0) {
        color = vec4(0.0, 0.0, 1.0, 1.0); // blue for ocean
    } else if (Biome == 1) {
        color = vec4(1.0, 1.0, 0.0, 1.0); // yellow for beach
    } else if (Biome == 2) {
        color = vec4(0.0, 1.0, 0.0, 1.0); // green for forest
    } else if (Biome == 3) {
        color = vec4(0.5, 0.5, 0.5, 1.0); // gray for mountain
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0); // white for snow
    }
    */

    //float h = (Height + 16)/64.0f;
    //FragColor = vec4(h, h, h, 1.0);
    float OceanHeight = -16.0;
    float SnowHeight = 48.0;
    float BiomeFactor = (Height - OceanHeight) / (SnowHeight - OceanHeight);

    // Compute biome
    float Biome = mix(0.0, 4.0, BiomeFactor);

    // Compute color based on biome
    vec3 color;
    if (Biome < 1.0) {
        color = vec3(0.0, 0.0, 1.0); // blue for ocean
    } else if (Biome < 2.0) {
        color = vec3(1.0, 1.0, 0.0); // yellow for beach
    } else if (Biome < 3.0) {
        color = vec3(0.0, 1.0, 0.0); // green for forest
    } else if (Biome < 4.0) {
        color = vec3(0.5, 0.5, 0.5); // gray for mountain
    } else {
        color = vec3(1.0, 1.0, 1.0); // white for snow
    }
    FragColor = vec4(color, 1.0);
}