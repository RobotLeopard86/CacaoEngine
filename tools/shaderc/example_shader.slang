import cacaoshaderbase;

struct Passthru {
    float4 pos : SV_Position;
    float happy;
};

[shader("vertex")] Passthru VS_main(VSIn input, uniform float4x4 transform) {
    Passthru o = {};
    o.pos = mul(cacao.projection, mul(cacao.view, mul(transform, float4(input.position, 1.0))));
    o.happy = (o.pos.x * o.pos.y) + 3.14f - (o.pos.z / 2.1f);
    return o;
}

[shader("fragment")] float4 FS_main(Passthru p) : SV_Target {
    return float4(p.pos.r * 0.9f + 0.31f, p.pos.g, p.pos.b / 2, p.happy);
}