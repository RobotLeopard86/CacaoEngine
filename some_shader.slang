import cacaoshaderbase;

[shader("vertex")] float4 VS_main(VSIn input) : SV_Position {
    return mul(float4(input.position, 1.0), mul(transform, mul(cacao.view, cacao.projection)));
}

[shader("fragment")] float4 FS_main(float4 pos : SV_Position) : SV_Target {
    return float4(pos.r * 0.9 + 0.31, pos.g, pos.b / 2, 0.586);
}