uniform sampler2D inp;
uniform float test;
void main()
{
	gl_FragColor = texture2D(inp, vec2(gl_TexCoord[0])) + test;
}
