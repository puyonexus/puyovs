R"(
uniform vec4 cl;
uniform float time;
uniform sampler2D tex;
void main(void) {
    vec2 ccord = gl_TexCoord[0].xy;
    vec2 pcord;
    vec2 final;
    float zoomspeed=0.5;
    float rotatespeed=0.25;
    ccord.x = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.x)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.x);
    ccord.y = step(0.5,ccord.y)*(-1.0+ 2.0*ccord.y)-(1.0-step(0.5,ccord.y))*(-1.0+ 2.0*ccord.y);
    pcord.x = 0.1/sqrt(ccord.x*ccord.x+ccord.y*ccord.y);pcord.y = atan(ccord.y,ccord.x)/3.141592;
    final.x = step(0.25,pcord.x)*mod(pcord.x+zoomspeed*time,0.5)+(1.0-step(0.25,pcord.x))*mod(pcord.x+zoomspeed*time,0.5);
    final.y = step(0.25,pcord.y)*mod(pcord.y+rotatespeed*time,0.5)+(1.0-step(0.25,pcord.y))*mod(pcord.y+rotatespeed*time,0.5);
    vec3 col = texture2D(tex,final).xyz;
    gl_FragColor = vec4(max(col/((pcord/0.1).x),0.1), 1.0) * cl;
};
)"