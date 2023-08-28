SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
  SHELLTYPE := msdos
endif

ifndef target
	target=$(shell $(cpp) -dumpmachine)
endif

.PHONY: default build link prebuild dirs

INCLUDES := -I../libs/assimp/include -I../libs/assimp/code -I../libs/assimp/ -I../libs/assimp/contrib -I../libs/assimp/contrib/pugixml/src -I../libs/assimp/contrib/rapidjson/include -I../libs/assimp/contrib/unzip -I../libs/assimp/contrib/openddlparser/include -I../assimp_generated
DEFINES :=
LIBS :=
LIBDIRS := -L/usr/lib64
MISCOPTS := -std=gnu++20 -c
MISCOPTSC := -std=gnu17 -c
CFGFLAGS :=

SRCDIR := ../libs/assimp/code
SRCDIR2 := ../libs/assimp/contrib
OBJDIR := ../Build/modules/assimp/$(target)/$(config)/obj
BINDIR := ../Build/modules/assimp/$(target)/$(config)/bin
BIN := $(BINDIR)/libassimp.a

ifeq ($(config),debug)
	CFGFLAGS += -g
else ifeq ($(config),release)
	CFGFLAGS += -O2
endif

OPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTS) $(CFGFLAGS)
COPTS := $(INCLUDES) $(DEFINES) $(LIBDIRS) $(LIBS) $(MISCOPTSC) $(CFGFLAGS)

OBJECTS := $(OBJDIR)/PostStepRegistry.o $(OBJDIR)/Assimp.o $(OBJDIR)/BaseImporter.o $(OBJDIR)/ImporterRegistry.o $(OBJDIR)/BaseProcess.o $(OBJDIR)/DefaultIOSystem.o $(OBJDIR)/DefaultIOStream.o $(OBJDIR)/ZipArchiveIOSystem.o $(OBJDIR)/Importer.o $(OBJDIR)/IOSystem.o $(OBJDIR)/SGSpatialSort.o $(OBJDIR)/Compression.o $(OBJDIR)/VertexTriangleAdjacency.o $(OBJDIR)/SpatialSort.o $(OBJDIR)/SceneCombiner.o $(OBJDIR)/ScenePreprocessor.o $(OBJDIR)/SkeletonMeshBuilder.o $(OBJDIR)/StandardShapes.o $(OBJDIR)/TargetAnimation.o $(OBJDIR)/RemoveComments.o $(OBJDIR)/Subdivision.o $(OBJDIR)/scene.o $(OBJDIR)/Bitmap.o $(OBJDIR)/Version.o $(OBJDIR)/CreateAnimMesh.o $(OBJDIR)/simd.o $(OBJDIR)/material.o $(OBJDIR)/AssertHandler.o $(OBJDIR)/Exceptional.o $(OBJDIR)/Base64.o $(OBJDIR)/GeometryUtils.o $(OBJDIR)/CInterfaceIOWrapper.o $(OBJDIR)/DefaultLogger.o $(OBJDIR)/CalcTangentsProcess.o $(OBJDIR)/ComputeUVMappingProcess.o $(OBJDIR)/ConvertToLHProcess.o $(OBJDIR)/EmbedTexturesProcess.o $(OBJDIR)/FindDegenerates.o $(OBJDIR)/FindInstancesProcess.o $(OBJDIR)/FindInvalidDataProcess.o $(OBJDIR)/FixNormalsStep.o $(OBJDIR)/DropFaceNormalsProcess.o $(OBJDIR)/GenFaceNormalsProcess.o $(OBJDIR)/GenVertexNormalsProcess.o $(OBJDIR)/PretransformVertices.o $(OBJDIR)/ImproveCacheLocality.o $(OBJDIR)/JoinVerticesProcess.o $(OBJDIR)/LimitBoneWeightsProcess.o $(OBJDIR)/RemoveRedundantMaterials.o $(OBJDIR)/RemoveVCProcess.o $(OBJDIR)/SortByPTypeProcess.o $(OBJDIR)/SplitLargeMeshes.o $(OBJDIR)/TextureTransform.o $(OBJDIR)/TriangulateProcess.o $(OBJDIR)/ValidateDataStructure.o $(OBJDIR)/OptimizeGraph.o $(OBJDIR)/OptimizeMeshes.o $(OBJDIR)/DeboneProcess.o $(OBJDIR)/ProcessHelper.o $(OBJDIR)/MakeVerboseFormat.o $(OBJDIR)/ScaleProcess.o $(OBJDIR)/ArmaturePopulate.o $(OBJDIR)/GenBoundingBoxesProcess.o $(OBJDIR)/SplitByBoneCountProcess.o $(OBJDIR)/MaterialSystem.o $(OBJDIR)/STEPFileReader.o $(OBJDIR)/STEPFileEncoding.o $(OBJDIR)/FBXImporter.o $(OBJDIR)/FBXParser.o $(OBJDIR)/FBXTokenizer.o $(OBJDIR)/FBXConverter.o $(OBJDIR)/FBXUtil.o $(OBJDIR)/FBXDocument.o $(OBJDIR)/FBXProperties.o $(OBJDIR)/FBXMeshGeometry.o $(OBJDIR)/FBXMaterial.o $(OBJDIR)/FBXModel.o $(OBJDIR)/FBXNodeAttribute.o $(OBJDIR)/FBXAnimation.o $(OBJDIR)/FBXDeformer.o $(OBJDIR)/FBXBinaryTokenizer.o $(OBJDIR)/FBXDocumentUtil.o $(OBJDIR)/contrib/ioapi.o $(OBJDIR)/contrib/unzip.o $(OBJDIR)/contrib/shapes.o $(OBJDIR)/contrib/advancing_front.o $(OBJDIR)/contrib/cdt.o $(OBJDIR)/contrib/sweep.o $(OBJDIR)/contrib/sweep_context.o $(OBJDIR)/contrib/clipper.o $(OBJDIR)/contrib/OpenDDLParser.o $(OBJDIR)/contrib/DDLNode.o $(OBJDIR)/contrib/OpenDDLCommon.o $(OBJDIR)/contrib/OpenDDLExport.o $(OBJDIR)/contrib/Value.o $(OBJDIR)/contrib/OpenDDLStream.o $(OBJDIR)/contrib/o3dgcArithmeticCodec.o $(OBJDIR)/contrib/o3dgcDynamicVectorDecoder.o $(OBJDIR)/contrib/o3dgcDynamicVectorEncoder.o $(OBJDIR)/contrib/o3dgcTools.o $(OBJDIR)/contrib/o3dgcTriangleFans.o

default:
	@echo "Citrus Engine assimp Module Builder"
	@echo "========================================="
	@echo ""
	@echo "Do not run directly. This should only be invoked by the modulebuild.mk Makefile."

dirs:
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(OBJDIR)
else
	@mkdir $(subst /,\\,$(OBJDIR))
endif
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(OBJDIR)/contrib
else
	@mkdir $(subst /,\\,$(OBJDIR)/contrib)
endif
ifeq (posix,$(SHELLTYPE))
	@mkdir -p $(BINDIR)
else
	@mkdir $(subst /,\\,$(BINDIR))
endif

prebuild:
	@echo "Module 'assimp' build: Compile Objects (1/2)"

link:
	@echo ""
	@echo "Module 'assimp' build: Link Library (2/2)"
	@echo "Linking '$(subst ../,,$(BIN))'..."
	@$(AR) -rcs $(BIN) $(OBJECTS)

build: dirs prebuild $(OBJECTS) link
	@:

# Files
###########

$(OBJDIR)/PostStepRegistry.o: $(SRCDIR)/Common/PostStepRegistry.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Assimp.o: $(SRCDIR)/Common/Assimp.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/BaseImporter.o: $(SRCDIR)/Common/BaseImporter.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ImporterRegistry.o: $(SRCDIR)/Common/ImporterRegistry.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/BaseProcess.o: $(SRCDIR)/Common/BaseProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/DefaultIOSystem.o: $(SRCDIR)/Common/DefaultIOSystem.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/DefaultIOStream.o: $(SRCDIR)/Common/DefaultIOStream.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ZipArchiveIOSystem.o: $(SRCDIR)/Common/ZipArchiveIOSystem.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Importer.o: $(SRCDIR)/Common/Importer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/IOSystem.o: $(SRCDIR)/Common/IOSystem.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SGSpatialSort.o: $(SRCDIR)/Common/SGSpatialSort.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Compression.o: $(SRCDIR)/Common/Compression.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/VertexTriangleAdjacency.o: $(SRCDIR)/Common/VertexTriangleAdjacency.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SpatialSort.o: $(SRCDIR)/Common/SpatialSort.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SceneCombiner.o: $(SRCDIR)/Common/SceneCombiner.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ScenePreprocessor.o: $(SRCDIR)/Common/ScenePreprocessor.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SkeletonMeshBuilder.o: $(SRCDIR)/Common/SkeletonMeshBuilder.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/StandardShapes.o: $(SRCDIR)/Common/StandardShapes.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/TargetAnimation.o: $(SRCDIR)/Common/TargetAnimation.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/RemoveComments.o: $(SRCDIR)/Common/RemoveComments.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Subdivision.o: $(SRCDIR)/Common/Subdivision.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/scene.o: $(SRCDIR)/Common/scene.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Bitmap.o: $(SRCDIR)/Common/Bitmap.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Version.o: $(SRCDIR)/Common/Version.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/CreateAnimMesh.o: $(SRCDIR)/Common/CreateAnimMesh.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/simd.o: $(SRCDIR)/Common/simd.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/material.o: $(SRCDIR)/Common/material.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/AssertHandler.o: $(SRCDIR)/Common/AssertHandler.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Exceptional.o: $(SRCDIR)/Common/Exceptional.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/Base64.o: $(SRCDIR)/Common/Base64.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GeometryUtils.o: $(SRCDIR)/Geometry/GeometryUtils.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/CInterfaceIOWrapper.o: $(SRCDIR)/CApi/CInterfaceIOWrapper.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/DefaultLogger.o: $(SRCDIR)/Common/DefaultLogger.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/CalcTangentsProcess.o: $(SRCDIR)/PostProcessing/CalcTangentsProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ComputeUVMappingProcess.o: $(SRCDIR)/PostProcessing/ComputeUVMappingProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ConvertToLHProcess.o: $(SRCDIR)/PostProcessing/ConvertToLHProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/EmbedTexturesProcess.o: $(SRCDIR)/PostProcessing/EmbedTexturesProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FindDegenerates.o: $(SRCDIR)/PostProcessing/FindDegenerates.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FindInstancesProcess.o: $(SRCDIR)/PostProcessing/FindInstancesProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FindInvalidDataProcess.o: $(SRCDIR)/PostProcessing/FindInvalidDataProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FixNormalsStep.o: $(SRCDIR)/PostProcessing/FixNormalsStep.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/DropFaceNormalsProcess.o: $(SRCDIR)/PostProcessing/DropFaceNormalsProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GenFaceNormalsProcess.o: $(SRCDIR)/PostProcessing/GenFaceNormalsProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GenVertexNormalsProcess.o: $(SRCDIR)/PostProcessing/GenVertexNormalsProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/PretransformVertices.o: $(SRCDIR)/PostProcessing/PretransformVertices.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ImproveCacheLocality.o: $(SRCDIR)/PostProcessing/ImproveCacheLocality.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/JoinVerticesProcess.o: $(SRCDIR)/PostProcessing/JoinVerticesProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/LimitBoneWeightsProcess.o: $(SRCDIR)/PostProcessing/LimitBoneWeightsProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/RemoveRedundantMaterials.o: $(SRCDIR)/PostProcessing/RemoveRedundantMaterials.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/RemoveVCProcess.o: $(SRCDIR)/PostProcessing/RemoveVCProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SortByPTypeProcess.o: $(SRCDIR)/PostProcessing/SortByPTypeProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SplitLargeMeshes.o: $(SRCDIR)/PostProcessing/SplitLargeMeshes.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/TextureTransform.o: $(SRCDIR)/PostProcessing/TextureTransform.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/TriangulateProcess.o: $(SRCDIR)/PostProcessing/TriangulateProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ValidateDataStructure.o: $(SRCDIR)/PostProcessing/ValidateDataStructure.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OptimizeGraph.o: $(SRCDIR)/PostProcessing/OptimizeGraph.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/OptimizeMeshes.o: $(SRCDIR)/PostProcessing/OptimizeMeshes.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/DeboneProcess.o: $(SRCDIR)/PostProcessing/DeboneProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ProcessHelper.o: $(SRCDIR)/PostProcessing/ProcessHelper.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/MakeVerboseFormat.o: $(SRCDIR)/PostProcessing/MakeVerboseFormat.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ScaleProcess.o: $(SRCDIR)/PostProcessing/ScaleProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/ArmaturePopulate.o: $(SRCDIR)/PostProcessing/ArmaturePopulate.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/GenBoundingBoxesProcess.o: $(SRCDIR)/PostProcessing/GenBoundingBoxesProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/SplitByBoneCountProcess.o: $(SRCDIR)/PostProcessing/SplitByBoneCountProcess.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/MaterialSystem.o: $(SRCDIR)/Material/MaterialSystem.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/STEPFileReader.o: $(SRCDIR)/AssetLib/STEPParser/STEPFileReader.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/STEPFileEncoding.o: $(SRCDIR)/AssetLib/STEPParser/STEPFileEncoding.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXImporter.o: $(SRCDIR)/AssetLib/FBX/FBXImporter.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXParser.o: $(SRCDIR)/AssetLib/FBX/FBXParser.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXTokenizer.o: $(SRCDIR)/AssetLib/FBX/FBXTokenizer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXConverter.o: $(SRCDIR)/AssetLib/FBX/FBXConverter.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXUtil.o: $(SRCDIR)/AssetLib/FBX/FBXUtil.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXDocument.o: $(SRCDIR)/AssetLib/FBX/FBXDocument.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXProperties.o: $(SRCDIR)/AssetLib/FBX/FBXProperties.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXMeshGeometry.o: $(SRCDIR)/AssetLib/FBX/FBXMeshGeometry.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXMaterial.o: $(SRCDIR)/AssetLib/FBX/FBXMaterial.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXModel.o: $(SRCDIR)/AssetLib/FBX/FBXModel.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXNodeAttribute.o: $(SRCDIR)/AssetLib/FBX/FBXNodeAttribute.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXAnimation.o: $(SRCDIR)/AssetLib/FBX/FBXAnimation.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXDeformer.o: $(SRCDIR)/AssetLib/FBX/FBXDeformer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXBinaryTokenizer.o: $(SRCDIR)/AssetLib/FBX/FBXBinaryTokenizer.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/FBXDocumentUtil.o: $(SRCDIR)/AssetLib/FBX/FBXDocumentUtil.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/ioapi.o: $(SRCDIR2)/unzip/ioapi.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(COPTS) -o "$@" $<

$(OBJDIR)/contrib/unzip.o: $(SRCDIR2)/unzip/unzip.c
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(c) $(COPTS) -o "$@" $<

$(OBJDIR)/contrib/shapes.o: $(SRCDIR2)/poly2tri/poly2tri/common/shapes.cc
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/advancing_front.o: $(SRCDIR2)/poly2tri/poly2tri/sweep/advancing_front.cc
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/cdt.o: $(SRCDIR2)/poly2tri/poly2tri/sweep/cdt.cc
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/sweep.o: $(SRCDIR2)/poly2tri/poly2tri/sweep/sweep.cc
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/sweep_context.o: $(SRCDIR2)/poly2tri/poly2tri/sweep/sweep_context.cc
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/clipper.o: $(SRCDIR2)/clipper/clipper.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/OpenDDLParser.o: $(SRCDIR2)/openddlparser/code/OpenDDLParser.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/DDLNode.o: $(SRCDIR2)/openddlparser/code/DDLNode.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/OpenDDLCommon.o: $(SRCDIR2)/openddlparser/code/OpenDDLCommon.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/OpenDDLExport.o: $(SRCDIR2)/openddlparser/code/OpenDDLExport.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/Value.o: $(SRCDIR2)/openddlparser/code/Value.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/OpenDDLStream.o: $(SRCDIR2)/openddlparser/code/OpenDDLStream.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/o3dgcArithmeticCodec.o: $(SRCDIR2)/Open3DGC/o3dgcArithmeticCodec.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/o3dgcDynamicVectorDecoder.o: $(SRCDIR2)/Open3DGC/o3dgcDynamicVectorDecoder.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/o3dgcDynamicVectorEncoder.o: $(SRCDIR2)/Open3DGC/o3dgcDynamicVectorEncoder.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/o3dgcTools.o: $(SRCDIR2)/Open3DGC/o3dgcTools.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<

$(OBJDIR)/contrib/o3dgcTriangleFans.o: $(SRCDIR2)/Open3DGC/o3dgcTriangleFans.cpp
	@echo "Compiling object $(notdir $@)... (source: $(notdir $<))"
	@$(cpp) $(OPTS) -o "$@" $<