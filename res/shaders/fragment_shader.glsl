#version 330 core

uniform sampler1D palette;
uniform float mand_iter;

in  vec2 c;

out vec4 FragColor;

void main()
{
    vec2 o = vec2(0.0, 0.0);
    vec2 z = vec2(0.0, 0.0);

    int n = 0;

    while(n < mand_iter && z.x*z.x+z.y*z.y <= 4)
    {

        o.x = z.x * z.x - z.y * z.y;
        o.y = 2*z.x*z.y;

        z = o;
        z += c;

        n++;
    }

    if(n == mand_iter){
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }else{
        FragColor = texture(palette, n/float(mand_iter)).rgba;
    }
}