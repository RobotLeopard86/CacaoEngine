project('assimp', 'c', 'cpp', meson_version: '>=1.10.0', version: '6.0.4', license: 'BSD-3-Clause', default_options: ['cpp_std=c++20', 'warning_level=3', 'b_staticpic=true', 'default_library=static'])

###################################
# COMMON SETUP
###################################

# Compiler settings
cpp = meson.get_compiler('cpp')
argsyntax = cpp.get_argument_syntax()
if cpp.get_id() == 'gcc'
	add_project_arguments(['-Wno-maybe-uninitialized', '-Wno-dangling-reference', '-fvisibility=hidden', '-fno-strict-aliasing', '-Wno-array-bounds', '-Wno-stringop-overflow'], language: 'cpp')
	add_project_arguments('-fno-strict-aliasing', language: 'c')
elif cpp.get_id() == 'clang'
	add_project_arguments(['-Wno-deprecated-non-prototype', '-Wno-long-long', '-fvisibility=hidden', '-fno-strict-aliasing'], language: 'cpp')
	add_project_arguments(['-Wno-deprecated-non-prototype', '-fno-strict-aliasing'], language: 'c')
elif cpp.get_id() == 'clang-cl'
	add_project_arguments(['/bigobj', '-Wno-deprecated-non-prototype', '-Wno-long-long', '-fvisibility=hidden', '-fno-strict-aliasing'], language: 'cpp')
	add_project_arguments(['/bigobj', '-Wno-deprecated-non-prototype', '-fno-strict-aliasing'], language: 'c')
elif cpp.get_id() == 'msvc'
	add_project_arguments(['/MP', '/bigobj', '/wd4244', '/wd4351', '/source-charset:utf-8', '/utf-8'], language: ['c', 'cpp'])
endif

# Windows definitions
windows_defs = []
if host_machine.system() == 'windows'
    windows_defs += '-DUNICODE'
    windows_defs += '-D_UNICODE'
	windows_defs += '-D_DLL'
	windows_defs += '-D_MT'
	windows_defs += '-DWIN32_LEAN_AND_MEAN'
	windows_defs += '-D_CRT_SECURE_NO_WARNINGS'
	windows_defs += '-D_SCL_SECURE_NO_WARNINGS'
	if get_option('buildtype') == 'debug'
		windows_defs += '-D_DEBUG'
		windows_defs += '-D_ITERATOR_DEBUG_LEVEL=2'
        if argsyntax == 'gcc'
		    windows_defs += '-fms-runtime-lib=dll_dbg'
        else
            windows_defs += '/MDd'
        endif
	else
		if argsyntax == 'gcc'
		    windows_defs += '-fms-runtime-lib=dll'
        else
            windows_defs += '/MD'
        endif
	endif
    add_project_arguments(windows_defs, language: ['c', 'cpp'])
endif

# Platform-independent defines
defs = [
	'-DASSIMP_BUILD_NO_EXPORT',
	'-DASSIMP_BUILD_NO_C4D_IMPORTER',
	'-DASSIMP_BUILD_NO_OWN_ZLIB',
	'-DASSIMP_IMPORTER_GLTF_USE_OPEN3DGC=1',
	'-DOPENDDLPARSER_BUILD',
	'-DOPENDDL_STATIC_LIBARY',
	'-DP2T_STATIC_EXPORTS',
	'-DRAPIDJSON_NOMEMBERITERATORCLASS',
	'-DRAPIDJSON_HAS_STDSTRING=1'
]
add_project_arguments(defs, language: ['c', 'cpp'])

# Debug macro
if get_option('buildtype') == 'debug'
	add_project_arguments('-D_DEBUG', language: ['c', 'cpp'])
endif

# Configure include directories
includes = include_directories([
	'.', 'code', 'include', 'contrib'
])

###################################
# DISABLED IMPORTERS
###################################

disabled_importers = [
	'AMF', '3DS', 'AC', 'ASE', 'ASSBIN', 'B3D', 'BVH', 'DXF', 'CSM', 'HMP', 'IRRMESH', 'IQM',
	'IRR', 'LWO', 'LWS', 'M3D', 'MD2', 'MD3', 'MD5', 'MDC', 'MDL', 'NFF', 'NDO', 'OFF', 'OGRE',
	'OPENGEX', 'PLY', 'MS3D', 'COB', 'BLEND', 'IFC', 'XGL', 'Q3D', 'Q3BSP', 'RAW', 'SIB', 'SMD',
	'STL', 'TERRAGEN', '3D', 'USD', 'X', 'X3D', '3MF', 'MMD'
]
foreach di : disabled_importers
	add_project_arguments('-DASSIMP_BUILD_NO_' + di + '_IMPORTER', language: ['c', 'cpp'])
endforeach

###################################
# DEPENDENCIES
###################################

zlib_dep = dependency('zlib', static: true, required: true)
rt_dep = cpp.find_library('rt', static: true, required: host_machine.system() == 'linux')

pugixml = static_library('pugixml', sources: [
	'contrib' / 'pugixml' / 'src' / 'pugiconfig.hpp',
	'contrib' / 'pugixml' / 'src' / 'pugixml.hpp',
	'contrib' / 'pugixml' / 'src' / 'pugixml.cpp'
], include_directories: 'contrib' / 'pugixml' / 'src')
pugixml_dep = declare_dependency(include_directories: 'contrib' / 'pugixml' / 'src', link_whole: pugixml)

utf8cpp_dep = declare_dependency(include_directories: 'contrib' / 'utf8cpp' / 'source')

clipper = static_library('clipper', sources: [
	'contrib' / 'clipper' / 'clipper.hpp',
	'contrib' / 'clipper' / 'clipper.cpp'
], include_directories: 'contrib' / 'clipper')
clipper_dep = declare_dependency(include_directories: 'contrib' / 'clipper', link_whole: clipper)

poly2tri = static_library('poly2tri', sources: [
	'contrib' / 'poly2tri' / 'poly2tri' / 'common' / 'shapes.cc',
    'contrib' / 'poly2tri' / 'poly2tri' / 'common' / 'shapes.h',
    'contrib' / 'poly2tri' / 'poly2tri' / 'common' / 'utils.h',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'advancing_front.h',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'advancing_front.cc',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'cdt.cc',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'cdt.h',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'sweep.cc',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'sweep.h',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'sweep_context.cc',
    'contrib' / 'poly2tri' / 'poly2tri' / 'sweep' / 'sweep_context.h'
], include_directories: 'contrib' / 'poly2tri' / 'poly2tri')
poly2tri_dep = declare_dependency(include_directories: 'contrib' / 'poly2tri' / 'poly2tri', link_whole: poly2tri)

minzip = static_library('minzip', sources: [
	'contrib' / 'unzip' / 'unzip.h',
	'contrib' / 'unzip' / 'unzip.c',
	'contrib' / 'unzip' / 'ioapi.h',
	'contrib' / 'unzip' / 'ioapi.c',
	'contrib' / 'unzip' / 'crypt.h'
], include_directories: 'contrib' / 'unzip', dependencies: zlib_dep)
minzip_dep = declare_dependency(include_directories: 'contrib' / 'unzip', link_whole: minzip)

openddlparser = static_library('openddlparser', sources: [
	'contrib' / 'openddlparser' / 'code' / 'OpenDDLParser.cpp',
    'contrib' / 'openddlparser' / 'code' / 'DDLNode.cpp',
    'contrib' / 'openddlparser' / 'code' / 'OpenDDLCommon.cpp',
    'contrib' / 'openddlparser' / 'code' / 'OpenDDLExport.cpp',
    'contrib' / 'openddlparser' / 'code' / 'Value.cpp',
    'contrib' / 'openddlparser' / 'code' / 'OpenDDLStream.cpp',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'OpenDDLParser.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'OpenDDLParserUtils.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'OpenDDLCommon.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'OpenDDLExport.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'OpenDDLStream.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'DDLNode.h',
    'contrib' / 'openddlparser' / 'include' / 'openddlparser' / 'Value.h'
], include_directories: 'contrib' / 'openddlparser' / 'include')
openddlparser_dep = declare_dependency(include_directories: 'contrib' / 'openddlparser' / 'include', link_whole: openddlparser)

open3dgc = static_library('open3dgc', sources: [
	'contrib' / 'Open3DGC' / 'o3dgcAdjacencyInfo.h',
    'contrib' / 'Open3DGC' / 'o3dgcArithmeticCodec.cpp',
    'contrib' / 'Open3DGC' / 'o3dgcArithmeticCodec.h',
    'contrib' / 'Open3DGC' / 'o3dgcBinaryStream.h',
    'contrib' / 'Open3DGC' / 'o3dgcCommon.h',
    'contrib' / 'Open3DGC' / 'o3dgcDVEncodeParams.h',
    'contrib' / 'Open3DGC' / 'o3dgcDynamicVectorDecoder.cpp',
    'contrib' / 'Open3DGC' / 'o3dgcDynamicVectorDecoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcDynamicVectorEncoder.cpp',
    'contrib' / 'Open3DGC' / 'o3dgcDynamicVectorEncoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcDynamicVector.h',
    'contrib' / 'Open3DGC' / 'o3dgcFIFO.h',
    'contrib' / 'Open3DGC' / 'o3dgcIndexedFaceSet.h',
    'contrib' / 'Open3DGC' / 'o3dgcSC3DMCDecoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcSC3DMCEncodeParams.h',
    'contrib' / 'Open3DGC' / 'o3dgcSC3DMCEncoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcTimer.h',
    'contrib' / 'Open3DGC' / 'o3dgcTools.cpp',
    'contrib' / 'Open3DGC' / 'o3dgcTriangleFans.cpp',
    'contrib' / 'Open3DGC' / 'o3dgcTriangleFans.h',
    'contrib' / 'Open3DGC' / 'o3dgcTriangleListDecoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcTriangleListEncoder.h',
    'contrib' / 'Open3DGC' / 'o3dgcVector.h',
], include_directories: ['contrib' / 'Open3DGC', 'include'])
open3dgc_dep = declare_dependency(include_directories: 'contrib' / 'Open3DGC', link_whole: open3dgc)

rapidjson_dep = declare_dependency(include_directories: 'contrib' / 'rapidjson' / 'include')

stb_dep = declare_dependency(sources: 'contrib' / 'stb' / 'stb_image.h')

###################################
# GIT DATA
###################################

git = find_program('git', required: true)
git_branch = run_command([git, 'rev-parse', '--abbrev-ref', 'HEAD'], capture: true, check: true).stdout().strip()
git_hash = run_command([git, 'rev-parse', '--short=8', 'HEAD'], capture: true, check: true).stdout().strip()

###################################
# ASSIMP CONFIGURATION
###################################

conf_data = configuration_data()
conf_data.set10('ASSIMP_DOUBLE_PRECISION', false)
config_h = configure_file(input: 'include' / 'assimp' / 'config.h.in', output: 'config.h', format: 'cmake@', configuration: conf_data, build_subdir: 'assimp')

rev_data = configuration_data()
rev_data.set('GIT_COMMIT_HASH', git_hash)
rev_data.set('GIT_BRANCH', git_branch)
rev_data.set('ASSIMP_VERSION_MAJOR', 6)
rev_data.set('ASSIMP_VERSION_MINOR', 0)
rev_data.set('ASSIMP_VERSION_PATCH', 4)
rev_data.set('ASSIMP_PACKAGE_VERSION', 0)
rev_data.set('CMAKE_SHARED_LIBRARY_PREFIX', host_machine.system() == 'windows' ? '' : 'lib')
rev_data.set('LIBRARY_SUFFIX', '')
rev_data.set('CMAKE_DEBUG_POSTFIX', '')
revision_h = configure_file(input: 'include' / 'assimp' / 'revision.h.in', output: 'revision.h', format: 'cmake@', configuration: rev_data, build_subdir: 'assimp')

###################################
# SOURCE GROUPINGS
###################################

header_path = meson.project_source_root() / 'include' / 'assimp'
source_path = meson.project_source_root() / 'code'

compiler_headers = [
	header_path / 'Compiler' / 'pushpack1.h',
	header_path / 'Compiler' / 'poppack1.h',
	header_path / 'Compiler' / 'pstdint.h'
]

public_headers = [
	header_path /'anim.h',
	header_path /'aabb.h',
	header_path /'ai_assert.h',
	header_path /'camera.h',
	header_path /'color4.h',
	header_path /'ColladaMetaData.h',
	header_path /'commonMetaData.h',
	header_path /'defs.h',
	header_path /'cfileio.h',
	header_path /'light.h',
	header_path /'material.h',
	header_path /'matrix3x3.h',
	header_path /'matrix4x4.h',
	header_path /'mesh.h',
	header_path /'ObjMaterial.h',
	header_path /'pbrmaterial.h',
	header_path /'GltfMaterial.h',
	header_path /'postprocess.h',
	header_path /'quaternion.h',
	header_path /'scene.h',
	header_path /'metadata.h',
	header_path /'texture.h',
	header_path /'types.h',
	header_path /'vector2.h',
	header_path /'vector3.h',
	header_path /'version.h',
	header_path /'cimport.h',
	header_path /'AssertHandler.h',
	header_path /'importerdesc.h',
	header_path /'Importer.hpp',
	header_path /'DefaultLogger.hpp',
	header_path /'ProgressHandler.hpp',
	header_path /'IOStream.hpp',
	header_path /'IOSystem.hpp',
	header_path /'Logger.hpp',
	header_path /'LogStream.hpp',
	header_path /'NullLogger.hpp',
	header_path /'cexport.h',
	header_path /'Exporter.hpp',
	header_path /'DefaultIOStream.h',
	header_path /'DefaultIOSystem.h',
	header_path /'ZipArchiveIOSystem.h',
	header_path /'SceneCombiner.h',
	header_path /'fast_atof.h',
	header_path /'qnan.h',
	header_path /'BaseImporter.h',
	header_path /'Hash.h',
	header_path /'MemoryIOWrapper.h',
	header_path /'ParsingUtils.h',
	header_path /'StreamReader.h',
	header_path /'StreamWriter.h',
	header_path /'StringComparison.h',
	header_path /'StringUtils.h',
	header_path /'SGSpatialSort.h',
	header_path /'GenericProperty.h',
	header_path /'SpatialSort.h',
	header_path /'SkeletonMeshBuilder.h',
	header_path /'SmallVector.h',
	header_path /'SmoothingGroups.h',
	header_path /'StandardShapes.h',
	header_path /'RemoveComments.h',
	header_path /'Subdivision.h',
	header_path /'Vertex.h',
	header_path /'LineSplitter.h',
	header_path /'TinyFormatter.h',
	header_path /'Profiler.h',
	header_path /'LogAux.h',
	header_path /'Bitmap.h',
	header_path /'XMLTools.h',
	header_path /'IOStreamBuffer.h',
	header_path /'CreateAnimMesh.h',
	header_path /'XmlParser.h',
	header_path /'BlobIOSystem.h',
	header_path /'MathFunctions.h',
	header_path /'Exceptional.h',
	header_path /'ByteSwapper.h',
	header_path /'Base64.hpp',
	config_h,
	revision_h
]

core_src = [
	source_path / 'Common' / 'Assimp.cpp',
]

logging_src = [
	header_path / 'DefaultLogger.hpp',
	header_path / 'LogStream.hpp',
	header_path / 'Logger.hpp',
	header_path / 'NullLogger.hpp',
	source_path / 'Common' / 'Win32DebugLogStream.h',
	source_path / 'Common' / 'DefaultLogger.cpp',
	source_path / 'Common' / 'FileLogStream.h',
	source_path / 'Common' / 'StdOStreamLogStream.h'
]

common_src = [
	source_path / 'Common' / 'StbCommon.h',
	source_path / 'Common' / 'Compression.cpp',
	source_path / 'Common' / 'Compression.h',
	source_path / 'Common' / 'BaseImporter.cpp',
	source_path / 'Common' / 'BaseProcess.cpp',
	source_path / 'Common' / 'BaseProcess.h',
	source_path / 'Common' / 'Importer.h',
	source_path / 'Common' / 'ScenePrivate.h',
	source_path / 'Common' / 'PostStepRegistry.cpp',
	source_path / 'Common' / 'ImporterRegistry.cpp',
	source_path / 'Common' / 'DefaultProgressHandler.h',
	source_path / 'Common' / 'DefaultIOStream.cpp',
	source_path / 'Common' / 'IOSystem.cpp',
	source_path / 'Common' / 'DefaultIOSystem.cpp',
	source_path / 'Common' / 'ZipArchiveIOSystem.cpp',
	source_path / 'Common' / 'PolyTools.h',
	source_path / 'Common' / 'Maybe.h',
	source_path / 'Common' / 'Importer.cpp',
	source_path / 'Common' / 'IFF.h',
	source_path / 'Common' / 'SGSpatialSort.cpp',
	source_path / 'Common' / 'VertexTriangleAdjacency.cpp',
	source_path / 'Common' / 'VertexTriangleAdjacency.h',
	source_path / 'Common' / 'SpatialSort.cpp',
	source_path / 'Common' / 'SceneCombiner.cpp',
	source_path / 'Common' / 'ScenePreprocessor.cpp',
	source_path / 'Common' / 'ScenePreprocessor.h',
	source_path / 'Common' / 'SkeletonMeshBuilder.cpp',
	source_path / 'Common' / 'StackAllocator.h',
	source_path / 'Common' / 'StandardShapes.cpp',
	source_path / 'Common' / 'TargetAnimation.cpp',
	source_path / 'Common' / 'TargetAnimation.h',
	source_path / 'Common' / 'RemoveComments.cpp',
	source_path / 'Common' / 'Subdivision.cpp',
	source_path / 'Common' / 'scene.cpp',
	source_path / 'Common' / 'Bitmap.cpp',
	source_path / 'Common' / 'Version.cpp',
	source_path / 'Common' / 'CreateAnimMesh.cpp',
	source_path / 'Common' / 'simd.h',
	source_path / 'Common' / 'simd.cpp',
	source_path / 'Common' / 'material.cpp',
	source_path / 'Common' / 'AssertHandler.cpp',
	source_path / 'Common' / 'Exceptional.cpp',
	source_path / 'Common' / 'Base64.cpp'
]

capi_src = [
	source_path / 'CApi' / 'CInterfaceIOWrapper.cpp',
	source_path / 'CApi' / 'CInterfaceIOWrapper.h',
]

geometry_src = [
	source_path / 'Geometry/GeometryUtils.h',
	source_path / 'Geometry/GeometryUtils.cpp'
]

material_src = [
	source_path / 'Material' / 'MaterialSystem.cpp',
	source_path / 'Material' / 'MaterialSystem.h'
]

postprocess_src = [
	source_path / 'PostProcessing' / 'CalcTangentsProcess.cpp',
	source_path / 'PostProcessing' / 'CalcTangentsProcess.h',
	source_path / 'PostProcessing' / 'ComputeUVMappingProcess.cpp',
	source_path / 'PostProcessing' / 'ComputeUVMappingProcess.h',
	source_path / 'PostProcessing' / 'ConvertToLHProcess.cpp',
	source_path / 'PostProcessing' / 'ConvertToLHProcess.h',
	source_path / 'PostProcessing' / 'EmbedTexturesProcess.cpp',
	source_path / 'PostProcessing' / 'EmbedTexturesProcess.h',
	source_path / 'PostProcessing' / 'FindDegenerates.cpp',
	source_path / 'PostProcessing' / 'FindDegenerates.h',
	source_path / 'PostProcessing' / 'FindInstancesProcess.cpp',
	source_path / 'PostProcessing' / 'FindInstancesProcess.h',
	source_path / 'PostProcessing' / 'FindInvalidDataProcess.cpp',
	source_path / 'PostProcessing' / 'FindInvalidDataProcess.h',
	source_path / 'PostProcessing' / 'FixNormalsStep.cpp',
	source_path / 'PostProcessing' / 'FixNormalsStep.h',
	source_path / 'PostProcessing' / 'DropFaceNormalsProcess.cpp',
	source_path / 'PostProcessing' / 'DropFaceNormalsProcess.h',
	source_path / 'PostProcessing' / 'GenFaceNormalsProcess.cpp',
	source_path / 'PostProcessing' / 'GenFaceNormalsProcess.h',
	source_path / 'PostProcessing' / 'GenVertexNormalsProcess.cpp',
	source_path / 'PostProcessing' / 'GenVertexNormalsProcess.h',
	source_path / 'PostProcessing' / 'PretransformVertices.cpp',
	source_path / 'PostProcessing' / 'PretransformVertices.h',
	source_path / 'PostProcessing' / 'ImproveCacheLocality.cpp',
	source_path / 'PostProcessing' / 'ImproveCacheLocality.h',
	source_path / 'PostProcessing' / 'JoinVerticesProcess.cpp',
	source_path / 'PostProcessing' / 'JoinVerticesProcess.h',
	source_path / 'PostProcessing' / 'LimitBoneWeightsProcess.cpp',
	source_path / 'PostProcessing' / 'LimitBoneWeightsProcess.h',
	source_path / 'PostProcessing' / 'RemoveRedundantMaterials.cpp',
	source_path / 'PostProcessing' / 'RemoveRedundantMaterials.h',
	source_path / 'PostProcessing' / 'RemoveVCProcess.cpp',
	source_path / 'PostProcessing' / 'RemoveVCProcess.h',
	source_path / 'PostProcessing' / 'SortByPTypeProcess.cpp',
	source_path / 'PostProcessing' / 'SortByPTypeProcess.h',
	source_path / 'PostProcessing' / 'SplitLargeMeshes.cpp',
	source_path / 'PostProcessing' / 'SplitLargeMeshes.h',
	source_path / 'PostProcessing' / 'TextureTransform.cpp',
	source_path / 'PostProcessing' / 'TextureTransform.h',
	source_path / 'PostProcessing' / 'TriangulateProcess.cpp',
	source_path / 'PostProcessing' / 'TriangulateProcess.h',
	source_path / 'PostProcessing' / 'ValidateDataStructure.cpp',
	source_path / 'PostProcessing' / 'ValidateDataStructure.h',
	source_path / 'PostProcessing' / 'OptimizeGraph.cpp',
	source_path / 'PostProcessing' / 'OptimizeGraph.h',
	source_path / 'PostProcessing' / 'OptimizeMeshes.cpp',
	source_path / 'PostProcessing' / 'OptimizeMeshes.h',
	source_path / 'PostProcessing' / 'DeboneProcess.cpp',
	source_path / 'PostProcessing' / 'DeboneProcess.h',
	source_path / 'PostProcessing' / 'ProcessHelper.h',
	source_path / 'PostProcessing' / 'ProcessHelper.cpp',
	source_path / 'PostProcessing' / 'MakeVerboseFormat.cpp',
	source_path / 'PostProcessing' / 'MakeVerboseFormat.h',
	source_path / 'PostProcessing' / 'ScaleProcess.cpp',
	source_path / 'PostProcessing' / 'ScaleProcess.h',
	source_path / 'PostProcessing' / 'ArmaturePopulate.cpp',
	source_path / 'PostProcessing' / 'ArmaturePopulate.h',
	source_path / 'PostProcessing' / 'GenBoundingBoxesProcess.cpp',
	source_path / 'PostProcessing' / 'GenBoundingBoxesProcess.h',
	source_path / 'PostProcessing' / 'SplitByBoneCountProcess.cpp',
	source_path / 'PostProcessing' / 'SplitByBoneCountProcess.h'
]

importer_src = [
	# Collada
	source_path / 'AssetLib' / 'Collada' / 'ColladaHelper.cpp',
	source_path / 'AssetLib' / 'Collada' / 'ColladaHelper.h',
	source_path / 'AssetLib' / 'Collada' / 'ColladaLoader.cpp',
	source_path / 'AssetLib' / 'Collada' / 'ColladaLoader.h',
	source_path / 'AssetLib' / 'Collada' / 'ColladaParser.cpp',
	source_path / 'AssetLib' / 'Collada' / 'ColladaParser.h',

	# Wavefront OBJ
	source_path / 'AssetLib' / 'Obj' / 'ObjFileData.h',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileImporter.cpp',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileImporter.h',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileMtlImporter.cpp',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileMtlImporter.h',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileParser.cpp',
	source_path / 'AssetLib' / 'Obj' / 'ObjFileParser.h',
	source_path / 'AssetLib' / 'Obj' / 'ObjTools.h',

	# FBX
	source_path / 'AssetLib' / 'FBX' / 'FBXUtil.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXUtil.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXCommon.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXImporter.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXCompileConfig.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXImporter.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXParser.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXParser.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXTokenizer.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXTokenizer.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXImportSettings.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXConverter.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXConverter.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXDocument.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXDocument.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXProperties.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXProperties.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXMeshGeometry.h',
	source_path / 'AssetLib' / 'FBX' / 'FBXMeshGeometry.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXMaterial.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXModel.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXAnimation.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXNodeAttribute.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXDeformer.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXBinaryTokenizer.cpp',
	source_path / 'AssetLib' / 'FBX' / 'FBXDocumentUtil.cpp',

	# glTF(2)
	source_path / 'AssetLib' / 'glTFCommon' / 'glTFCommon.h',
	source_path / 'AssetLib' / 'glTFCommon' / 'glTFCommon.cpp',
	source_path / 'AssetLib' / 'glTF' / 'glTFAsset.h',
	source_path / 'AssetLib' / 'glTF' / 'glTFAssetWriter.h',
	source_path / 'AssetLib' / 'glTF' / 'glTFImporter.cpp',
	source_path / 'AssetLib' / 'glTF' / 'glTFImporter.h',
	source_path / 'AssetLib' / 'glTF2' / 'glTF2Asset.h',
	source_path / 'AssetLib' / 'glTF2' / 'glTF2AssetWriter.h',
	source_path / 'AssetLib' / 'glTF2' / 'glTF2Importer.cpp',
	source_path / 'AssetLib' / 'glTF2' / 'glTF2Importer.h'
]

###################################
# ASSIMP LIBRARY
###################################

assimp = static_library('assimp', sources: [
	core_src, capi_src, common_src, compiler_headers,
	geometry_src, logging_src, importer_src,
	postprocess_src, material_src, public_headers
], include_directories: includes, dependencies: [
	zlib_dep, rt_dep, pugixml_dep, utf8cpp_dep, clipper_dep,
	poly2tri_dep, minzip_dep, openddlparser_dep, open3dgc_dep,
	rapidjson_dep, stb_dep
])

assimp_dep = declare_dependency(include_directories: include_directories('include', '.'), link_with: assimp)