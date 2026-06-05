namespace Cacao {
	struct VSIn3D {
		float3 position : Position0;
		float2 texCoords : TexCoord0;
		float3 normal : Normal0;
		float3 tangent : Tangent0;
		float3 bitangent : Bitangent0;
	};

	struct VSIn2D {
		float2 position : Position0;
		float2 texCoords : TexCoord0;
	};

	struct GlobalData {
		float4x4 viewProjection;
	};
	ParameterBlock<GlobalData> globals;

	[vk::push_constant]
	ConstantBuffer<float4x4> transform;
}
