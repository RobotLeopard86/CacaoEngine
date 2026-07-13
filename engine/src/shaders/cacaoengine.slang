namespace Cacao {
	struct VSIn3D {
		float3 position : Position0;
		float2 texCoords : TexCoord0;
		float3 normal : Normal0;
		float4 tangent : Tangent0;
	};

	struct SurfaceInput {
		float2 texCoords;
		float3 localPos;
		float3 worldPos;
		float3 normal;
		float4 tangent;
	}

	struct VSOut3D {
		float4 gpuPosition : SV_Position;
		SurfaceInput surfaceIn;
	};

	struct VSIn2D {
		float2 position : Position0;
		float2 texCoords : TexCoord0;
	};

	struct CanvasInput {
		float2 position;
		float2 texCoords;
	};

	struct VSOut2D {
		float4 gpuPosition : SV_Position;
		CanvasInput canvasIn;
	};

	enum RenderMode : uint32_t {
		Opaque = 0,
		Cutout = 1,
		Transparent = 2
	}

	struct GlobalData {
		float4x4 viewMatrix;
		float4x4 projectionMatrix;
		float4x4 viewProjectionMatrix;
		float4 cameraWorldRotationQuat;
		float3 cameraWorldPosition;
		float worldTime;
		float deltaTime;
	};

	[vk::binding(0)]
	ConstantBuffer<GlobalData> globals : register(b0);

	struct Transform {
		float4x4 transformMatrix;
		float3x3 normalMatrix;
		float handedness;
	};

	[vk::push_constant]
	ConstantBuffer<Transform> transform : register(b1);

	struct RMWrap {
		RenderMode mode;
	}

	[vk::push_constant]
	ConstantBuffer<RMWrap> renderMode : register(b2);

	struct SurfaceSample {
		float4 albedo;
	};
}
