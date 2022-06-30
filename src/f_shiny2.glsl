#version 330


out vec4 pixelColor;
in vec4 l1;
in vec4 l2;
in vec4 n;
in vec4 v;
in vec2 iTexCoord;

in vec2 i_tc;
uniform sampler2D tex;

void main(void) {



	vec4 mn = normalize(n);
	vec4 mv = normalize(v);

	vec4 ml1 = normalize(l1);
	vec4 ml2 = normalize(l2);

	vec4 mr1 = reflect(-ml1, mn);
	vec4 mr2 = reflect(-ml2, mn);


	vec4 kd = texture(tex, vec2(iTexCoord.x, -iTexCoord.y)); 
	vec4 ks = vec4(0.1, 0.1, 0.1, 1);

	float nl1 = pow(clamp(dot(mn, ml1), 0, 1), 1);
	float rv1 = pow(clamp(dot(mr1, mv), 0, 1), 25);

	float nl2 = pow(clamp(dot(mn, ml2), 0, 1), 1);
	float rv2 = pow(clamp(dot(mr2, mv), 0, 1), 25);


	float nl_sum = max(nl1,nl2);
	float rv_sum = max(rv1,rv2);

	pixelColor= vec4(kd.rgb * (nl_sum), 1) + vec4(ks.rgb*(rv_sum), 0);

}
