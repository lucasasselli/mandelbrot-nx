#version 410 core

uniform sampler1D palette;
uniform double mand_iter;
uniform dvec2 offset;
uniform double zoom;


in  vec2 pos;

out vec4 FragColor;

void main()
{
    dvec2 o = dvec2(0.0, 0.0);
    dvec2 z = dvec2(0.0, 0.0);

    dvec2 c = dvec2(pos)*zoom + offset;

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