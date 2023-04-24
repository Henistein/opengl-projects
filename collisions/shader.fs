#version 410 core

in float Height;
in vec2 TexCoord;
uniform int option;

out vec4 FragColor;

vec4 biome_summer(float h){
    vec4 color;
    if (h < 0.1) {
        color = vec4(0.0, 0.0, h * 5.0+0.2, 1.0); // water
    } else if (h < 0.11) {
        color = vec4(h * 5.0, h * 5.0, 0.0, 1.0); // sand
    } else if (h < 0.2) {
        color = vec4(0.0, h * 3.0, 0.0, 1.0); // grassland
    } else if (h < 0.5) {
        color = vec4(h*0.7, h*0.7, h*0.7, 1.0); // rock/mountain
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0); // snow
    }
    return color;
}

vec4 biome_autumn(float h){
    vec4 color;
    if (h < 0.1) {
        color = vec4(0.0, 0.0, h * 5.0+0.2, 1.0); // water
    } else if (h < 0.11) {
        color = vec4(h * 5.0, h * 5.0, 0.0, 1.0); // sand
    } else if (h < 0.2) {
        color = vec4(h*0.9*5, h*0.4*5, 0.0, 1.0); // orange-brown soil
    } else if (h < 0.5) {
        color = vec4(h * 0.9, h * 0.6, 0.0, 1.0); // autumn leaves on ground
    } else {
        color = vec4(0.8, 0.8, 0.8, 1.0); // rock/mountain
    }
    return color;
}

vec4 biome_tundra(float h){
    vec4 color;
    if (h < 0.1) {
        color = vec4(0.6, 0.8, 0.9, 1.0) * (h * 5.0 + 0.2); // light blue sea/ice
    } else if (h < 0.2) {
        color = vec4(0.7, 0.8, 0.9, 1.0) * (h * 5.0 + 0.2); // light blue-gray soil
    } else if (h < 0.5) {
        color = vec4(0.8, 0.9, 1.0, 1.0) * (h * 1.5); // light gray-blue rock/mountain
    } else if (h < 0.55) {
        color = vec4(0.8, 0.8, 0.8, 1.0) * (h * 10.0); // light gray snow
    } else {
        color = vec4(1.0, 1.0, 1.0, 1.0) * (h * 10.0); // white snow
    }
    return color;
}

void main()
{
    float h = (Height + 16)/64.0f;

    vec4 color;
    if(option == 0){
        color = biome_summer(h);
    }
    else if(option == 1){
        color = biome_autumn(h);
    }
    else if(option == 2) {
        color = biome_tundra(h);
    }
    else{
        color = vec4(h, h, h, 1.0);  
    }


    FragColor = color;
}