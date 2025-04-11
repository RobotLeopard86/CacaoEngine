#version 450
layout(column_major) uniform;
layout(column_major) buffer;

#line 11575 0
struct _MatrixStorage_float4x4std140_0
{
    vec4  data_0[4];
};


#line 11 1
struct CacaoGlobalData_std140_0
{
    _MatrixStorage_float4x4std140_0 projection_0;
    _MatrixStorage_float4x4std140_0 view_0;
};


#line 13
layout(binding = 1)
layout(std140) uniform block_CacaoGlobalData_std140_0
{
    _MatrixStorage_float4x4std140_0 projection_0;
    _MatrixStorage_float4x4std140_0 view_0;
}cacao_0;

#line 13
struct GlobalParams_std140_0
{
    _MatrixStorage_float4x4std140_0 transform_0;
};


#line 13
layout(binding = 0)
layout(std140) uniform block_GlobalParams_std140_0
{
    _MatrixStorage_float4x4std140_0 transform_0;
}globalParams_0;

#line 1
layout(location = 0)
in vec3 input_position_0;


#line 3 2
void main()
{

#line 3
    gl_Position = (((((((((mat4x4(cacao_0.projection_0.data_0[0][0], cacao_0.projection_0.data_0[0][1], cacao_0.projection_0.data_0[0][2], cacao_0.projection_0.data_0[0][3], cacao_0.projection_0.data_0[1][0], cacao_0.projection_0.data_0[1][1], cacao_0.projection_0.data_0[1][2], cacao_0.projection_0.data_0[1][3], cacao_0.projection_0.data_0[2][0], cacao_0.projection_0.data_0[2][1], cacao_0.projection_0.data_0[2][2], cacao_0.projection_0.data_0[2][3], cacao_0.projection_0.data_0[3][0], cacao_0.projection_0.data_0[3][1], cacao_0.projection_0.data_0[3][2], cacao_0.projection_0.data_0[3][3])) * (mat4x4(cacao_0.view_0.data_0[0][0], cacao_0.view_0.data_0[0][1], cacao_0.view_0.data_0[0][2], cacao_0.view_0.data_0[0][3], cacao_0.view_0.data_0[1][0], cacao_0.view_0.data_0[1][1], cacao_0.view_0.data_0[1][2], cacao_0.view_0.data_0[1][3], cacao_0.view_0.data_0[2][0], cacao_0.view_0.data_0[2][1], cacao_0.view_0.data_0[2][2], cacao_0.view_0.data_0[2][3], cacao_0.view_0.data_0[3][0], cacao_0.view_0.data_0[3][1], cacao_0.view_0.data_0[3][2], cacao_0.view_0.data_0[3][3]))))) * (mat4x4(globalParams_0.transform_0.data_0[0][0], globalParams_0.transform_0.data_0[0][1], globalParams_0.transform_0.data_0[0][2], globalParams_0.transform_0.data_0[0][3], globalParams_0.transform_0.data_0[1][0], globalParams_0.transform_0.data_0[1][1], globalParams_0.transform_0.data_0[1][2], globalParams_0.transform_0.data_0[1][3], globalParams_0.transform_0.data_0[2][0], globalParams_0.transform_0.data_0[2][1], globalParams_0.transform_0.data_0[2][2], globalParams_0.transform_0.data_0[2][3], globalParams_0.transform_0.data_0[3][0], globalParams_0.transform_0.data_0[3][1], globalParams_0.transform_0.data_0[3][2], globalParams_0.transform_0.data_0[3][3]))))) * (vec4(input_position_0, 1.0))));

#line 3
    return;
}

