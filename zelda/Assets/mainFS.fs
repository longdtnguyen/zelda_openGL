#version 330

struct LightSource {
    vec3 position;
};
struct Mat {
    vec3 kd;
    vec3 ks;
    float shine;
    float alpha;
    int status;
    int mode;
};

in VsOutFsIn {
    vec3 position_ES; // Eye-space position
    vec3 normal_ES;   // Eye-space normal
    vec2 uv_ES;       // 
    vec4 lightSpace_ES;
    flat int time;
    flat int umbra;
    flat int reflect;
    LightSource light;
} fs_in;

layout (location = 0) out vec4 fragColour;
layout (location = 1) out vec4 brightColour;

uniform Mat mat;
uniform sampler2D tex;      //texture colour
uniform sampler2D shadowMap;




//-------------------------------UMBRA ------------------------------
float ShadowCalculation(vec4 posLightSpace) {

    //homogenizing ... aka divide by w
    vec3 proj = posLightSpace.xyz / posLightSpace.w;

    //normalize
    proj = proj * 0.5 + 0.5;

    if (proj.x < 0.0) proj.x = 0.0;
    if (proj.y < 0.0) proj.y = 0.0;
    if (proj.x > 1.0) proj.x = 1.0;
    if (proj.y > 1.0) proj.y = 1.0;
    //get the closest Depth
    float closest = texture(shadowMap,proj.xy).z;
    float current = proj.z;

    vec3 lightDir = normalize(fs_in.light.position - fs_in.position_ES);

    float bias = max(0.05 *  (1.0 - dot(normalize(fs_in.normal_ES), lightDir)), 0.005);

    float shadow = 0.0;

    vec2 texSize = 1.0/textureSize(shadowMap,0);

    for (int i = -1;i <= 1;i++) {
        for (int j = -1;j <= 1;j++) {
            float pcfDepth = texture(shadowMap,proj.xy + vec2(i,j) + texSize).r;
            shadow += current - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if(proj.z > 1.0) shadow = 0;

    return shadow * 5.0;
}

//---------------------------Cel shading----------------------------- 
vec3 Cel(vec3 fragPosition, vec3 fragNormal) {
    LightSource light = fs_in.light;

     // Direction from fragment to light source.
    vec3 l = normalize(light.position - fragPosition);


    vec3 ambient = 0.3 * texture(tex,fs_in.uv_ES).rgb;
    float shadow;
    if(fs_in.umbra == 1) shadow = ShadowCalculation(fs_in.lightSpace_ES);
    else shadow = 0;
    vec3 retCol;
    if (mat.status  == 2) {
        retCol = mat.kd;
        ambient = vec3(1.0);
    }else {
        retCol = texture(tex,fs_in.uv_ES).rgb;

        //get light intensity
        float intensity = max(dot(l,fragNormal),0.0);
        
        if (intensity > 0.35 && intensity < 1)
            retCol = retCol * 0.7;
        else if (intensity > 0.2)
            retCol = retCol * 0.4;
        else if (intensity > 0.15)
            retCol = retCol * 0.4;
        else
            retCol = retCol * 0.2;
        
        retCol = ambient + retCol;
        if (mat.status == 1) retCol = texture(tex,fs_in.uv_ES).rgb * 0.6;
        if (mat.status == 3) retCol = (ambient + (1- shadow)) *texture(tex,fs_in.uv_ES).rgb;   //water
    }
    if (fs_in.reflect == 5) retCol *= 0.5;
    return retCol;

}

vec3 phongModel(vec3 fragPosition, vec3 fragNormal) {
    LightSource light = fs_in.light;

    // Direction from fragment to light source.
    vec3 l = normalize(light.position - fragPosition);

    // Direction from fragment to viewer (origin - fragPosition).
    vec3 v = normalize(-fragPosition.xyz);

    float n_dot_l = max(dot(fragNormal, l), 0.0);

    vec3 diffuse;
    diffuse = texture(tex,fs_in.uv_ES).rgb * n_dot_l;

    vec3 specular = vec3(0.0);

    if (n_dot_l > 0.0) {
        // Halfway vector.
        vec3 h = normalize(v + l);
        float n_dot_h = max(dot(fragNormal, h), 0.0);

        specular = mat.ks * pow(n_dot_h, mat.shine);
    }

    return (diffuse + specular);
}

void main() {
    float fin_alpha = mat.alpha;
    if (fs_in.reflect == 5) fin_alpha = 0.9;

    if (mat.mode == 1) fragColour = vec4(Cel(fs_in.position_ES,fs_in.normal_ES), fin_alpha);
    else fragColour = vec4(phongModel(fs_in.position_ES,fs_in.normal_ES), fin_alpha);
    brightColour = vec4(max(Cel(fs_in.position_ES,fs_in.normal_ES)- 150,vec3(0.0)),1.0);

}
