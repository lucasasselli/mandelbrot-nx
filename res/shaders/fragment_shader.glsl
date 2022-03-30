#version 330 core

uniform sampler1D palette;

uniform vec2 offset;
uniform float zoom;

in  vec2 pos;

out vec4 FragColor;

void main()
{
    vec2 o = vec2(0.0, 0.0);
    vec2 z = vec2(0.0, 0.0);
    vec2 c = vec2(pos.x*zoom, pos.y*zoom) + offset;

    int n = 0;

    while(n < 255 && z.x*z.x+z.y*z.y <= 4)
    {

        o.x = z.x * z.x - z.y * z.y;
        o.y = 2*z.x*z.y;

        z = o;
        z += c;

        n++;
    }

    if(n == 255){
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }else{
        FragColor = texture(palette, n/255.0).rgba;
    }
}