#version 410 core

uniform sampler1D palette;
uniform int mand_iter;
uniform dvec2 offset;
uniform double zoom;
uniform int supersample;
uniform float ratio;

in  vec2 pos;

out vec4 FragColor;

void main()
{
    int n_sum = 0;

    double ss_factor_x = dFdx(pos.x)/double(supersample);
    double ss_factor_y = dFdx(pos.y)/double(supersample);

    for(int i=0; i<supersample; i++){
        for(int j=0; j<supersample; j++){

            int n = 0;

            dvec2 c = dvec2(pos.x-ss_factor_x*i, (pos.y-ss_factor_y*j)*ratio)*zoom + offset;

            dvec2 o = dvec2(0.0, 0.0);
            dvec2 z = dvec2(0.0, 0.0);

            while(n < mand_iter && z.x*z.x+z.y*z.y <= 4)
            {
                o.x = z.x * z.x - z.y * z.y;
                o.y = 2*z.x*z.y;

                z = o;
                z += c;

                n++;
            }

            n_sum += n;
        }
    }

    n_sum /= supersample*supersample;

    if(n_sum >= mand_iter){
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }else{
        FragColor = texture(palette, n_sum/float(mand_iter)).rgba;
    }
}