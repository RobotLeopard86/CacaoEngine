namespace Cacao {
	struct VSIn3D {
		float3 position : Position0;
		float2 texCoords : TexCoord0;
		float3 normal : Normal0;
		float4 tangent : Tangent0;
	};

	struct VSIn2D {
		float2 position : Position0;
		float2 texCoords : TexCoord0;
	};

	struct SurfaceInput {
		float2 texCoords;
		float3 localPos;
		float3 worldPos;
		float3 normal;
		float4 tangent;
	}

	struct VSOut {
		float4 outPosition : SV_Position;
		SurfaceInput surfaceIn;
	};

	enum RenderMode : uint8_t {
		Opaque = 0,
		Cutout = 1,
		Transparent = 2
	}

	struct GlobalData {
		float worldTime;
		float deltaTime;
		RenderMode renderMode;
	};
	[vk::binding(0)] ParameterBlock<GlobalData> globals : register(b0);

	struct CameraData {
		float4x4 viewMatrix;
		float4x4 projectionMatrix;
		float4x4 viewProjectionMatrix;
		float3 worldPosition;
	};
	[vk::binding(1)] ParameterBlock<CameraData> cam : register(b1);

	struct Transform {
		float4x4 transformMatrix;
		float4x4 normalMatrix;
	};

	[vk::push_constant] ConstantBuffer<Transform> transform : register(b2);

	struct SurfaceSample {
		float4 albedo;
	};
}
