import cacaoengine;
import cacaousercode;

[shader("fragment")]
float4 Cacao_Surface_FSmain(Cacao::SurfaceSample input) : SV_Target {
	//Calculate base color
	//Maybe someday we'll have lighting here...
	//Wow it's such a fancy algorithm
	float4 color = input.albedo;

	//Render mode application
	switch(Cacao::globals.renderMode) {
		case Cacao::RenderMode::Opaque:
			return float4(color.r, color.g, color.b, 1.0);
		case Cacao::RenderMode::Cutout:
			if(color.a < 1.0) discard;
			return color;
		case Cacao::RenderMode::Transparent:
			return color;
	}
}

[shader("fragment")]
float4 Cacao_Canvas_FSmain(float4 input) : SV_Target {
	//Not really anything important here (for now)
	return input;
}
