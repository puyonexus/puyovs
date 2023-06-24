R"(
uniform sampler2D tex;
uniform float color;
void main()
{
   gl_FragColor=texture2D(tex,gl_TexCoord[0].xy)+vec4(color,color,color,0);
}
    )"