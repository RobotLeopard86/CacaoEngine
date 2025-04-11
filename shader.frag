#version 450
layout(column_major) uniform;
layout(column_major) buffer;

#line 3 0
layout(location = 0)
out vec4 entryPointParam_FS_main_0;


void main()
{

#line 7
    entryPointParam_FS_main_0 = vec4(gl_FragCoord.x * 0.89999997615814209 + 0.31000000238418579, gl_FragCoord.y, gl_FragCoord.z / 2.0, 0.58600002527236938);

#line 7
    return;
}

