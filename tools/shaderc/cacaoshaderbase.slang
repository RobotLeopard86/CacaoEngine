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

	struct SurfaceInput {
		float2 texCoords;
		float3 objLocalPos;
		float3 objWorldPos;
		float3 worldNormal;
		float3 worldTangent;
		float3 worldBitangent;
	}

	struct VSOut {
		float4 outPosition : SV_Position;
		SurfaceInput surfaceIn;
	};

	struct GlobalData {
		float worldTime;
		float deltaTime;
	};
	ParameterBlock<GlobalData> globals;

	struct CameraData {
		float4x4 viewMatrix;
		float4x4 projectionMatrix;
		float4x4 viewProjectionMatrix;
		float3 worldPosition;
	};
	ParameterBlock<CameraData> cam;

	struct Transform {
		float4x4 transformMatrix;
		float4x4 normalMatrix;
	};

	[vk::push_constant]
	ConstantBuffer<Transform> transform;

	struct SurfaceSample {
		float4 albedo;
	};
}