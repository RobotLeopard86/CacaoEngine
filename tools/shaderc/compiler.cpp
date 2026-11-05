#include "compiler.hpp"
#include "slang.h"
#include "toolutil.hpp"

#include "CheckException.hpp"
#include "libcacaoasset.hpp"

#include <exception>
#include <sstream>
#include <fstream>
#include <cstring>

CacaoShaderCompiler::CacaoShaderCompiler() {
	//Initialize global session
	CVLOG_NONL("Initializing Slang global session... ");
	SlangResult r = slang::createGlobalSession(gSession.writeRef());
	CheckException(r == SLANG_OK && gSession, "Failed to create Slang global session!");
	CVLOG("Done.");
}

std::pair<bool, std::string> CacaoShaderCompiler::compile(const std::filesystem::path& in, const std::filesystem::path& out) {
	CVLOG_SINGLE("Compiling " << in << ": ")

	//Create session
	CVLOG_NONL("\tCreating compiler session... ");
	ComPtr<slang::ISession> session;
	{
		slang::SessionDesc sessionDesc = {};
		slang::TargetDesc tgtDesc = {};
		std::vector<slang::CompilerOptionEntry> entries;
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::VulkanUseGLLayout;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::EmitSpirvDirectly;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::GenerateWholeProgram;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::MatrixLayoutColumn;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		{
			slang::CompilerOptionEntry opt = {};
			opt.name = slang::CompilerOptionName::VulkanUseEntryPointName;
			opt.value = slang::CompilerOptionValue {slang::CompilerOptionValueKind::Int, true, 0, nullptr, nullptr};
			entries.push_back(opt);
		}
		tgtDesc.format = SLANG_SPIRV;
		tgtDesc.profile = gSession->findProfile("spirv_1_3");
		sessionDesc.targets = &tgtDesc;
		sessionDesc.targetCount = 1;
		sessionDesc.compilerOptionEntries = entries.data();
		sessionDesc.compilerOptionEntryCount = entries.size();
		SlangResult r = gSession->createSession(sessionDesc, session.writeRef());
		CompileCheck(r == SLANG_OK && session, "Failed to create Slang session!");
	}
	CVLOG("Done.");

	constexpr const char* cacaoModuleSrc =
#include "cacaoengine.inc"
		;

	//Load Cacao Engine module
	CVLOG_NONL("\tLoading Cacao shader module... ");
	ComPtr<slang::IModule> cacaoModule;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		cacaoModule = session->loadModuleFromSourceString("cacaoengine", "cacaoengine.slang", cacaoModuleSrc, diagnosticsBlob.writeRef());
		if(!cacaoModule) {
			std::stringstream err;
			err << "Failed to load Cacao shader module";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Load source as module
	CVLOG_NONL("\tReading source file... ");
	std::ifstream srcStream(in);
	CompileCheck(srcStream.is_open(), "Failed to open source file!");
	std::string src(std::istreambuf_iterator<char>(srcStream), {});
	CVLOG("Done.");
	CVLOG_NONL("\tCompiling user shader module... ");
	ComPtr<slang::IModule> mod;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		mod = session->loadModuleFromSourceString(in.string().c_str(), nullptr, src.c_str(), diagnosticsBlob.writeRef());
		if(!mod) {
			std::stringstream err;
			err << "Failed to compile shader module";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.");

	//Serialize IR blob
	CVLOG_NONL("\tSerializing IR blob... ")
	ComPtr<ISlangBlob>
		irBlob;
	{
		SlangResult r = mod->serialize(irBlob.writeRef());
		if(r != SLANG_OK || !irBlob) {
			std::stringstream err;
			err << "Failed to serialize IR blob!";
			CompileCheck(false, err.str());
		}
	}
	libcacaoasset::Shader shader;
	shader.irCode = std::vector<unsigned char>(irBlob->getBufferSize());
	std::memcpy(shader.irCode.data(), irBlob->getBufferPointer(), irBlob->getBufferSize());
	CVLOG("Done.")

	//Link program without entry points to ensure full reflection functionality
	CVLOG_NONL("\tLinking shader program...")
	std::array<slang::IComponentType*, 2> componentTypes {
		cacaoModule,
		mod};
	ComPtr<slang::IComponentType> composed;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult r = session->createCompositeComponentType(componentTypes.data(), componentTypes.size(), composed.writeRef(), diagnosticsBlob.writeRef());
		if(r != SLANG_OK || !composed) {
			std::stringstream err;
			err << "Failed to compose shader program";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	ComPtr<slang::IComponentType> linked;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult r = composed->link(linked.writeRef(), diagnosticsBlob.writeRef());
		if(r != SLANG_OK || !linked) {
			std::stringstream err;
			err << "Failed to link shader program";
			if(diagnosticsBlob) {
				err << ":\n"
					<< (const char*)diagnosticsBlob->getBufferPointer();
			} else {
				err << "!";
			}
			CompileCheck(false, err.str());
		}
	}
	CVLOG("Done.")

	//Validate shader (we also generate the descriptor here but it's not a separate step because generation could find validation errors)
	CVLOG_NONL("\tValidating shader... ");
	slang::ProgramLayout* layout;
	{
		ComPtr<slang::IBlob> diagnosticsBlob;
		layout = linked->getLayout(0, diagnosticsBlob.writeRef());
		CompileCheck(layout != nullptr, std::format("Failed to obtain shader program layout!\nThis is not a problem with your shader, it is a BUG and should be reported.\n\nSlang provided message:\n{}",
											std::string((const char*)diagnosticsBlob->getBufferPointer(), diagnosticsBlob->getBufferSize())));
	}
	{
		//Get list of shader functions
		slang::FunctionReflection* pre3D = layout->findFunctionByName("CacaoVertexPreprocess");
		slang::FunctionReflection* post3D = layout->findFunctionByName("CacaoVertexPostprocess");
		slang::FunctionReflection* custom3D = layout->findFunctionByName("CacaoVertexCustom");
		slang::FunctionReflection* pre2D = layout->findFunctionByName("CacaoVertexPreprocess2D");
		slang::FunctionReflection* custom2D = layout->findFunctionByName("CacaoVertexCustom2D");
		slang::FunctionReflection* surface = layout->findFunctionByName("CacaoSurfaceMain");
		slang::FunctionReflection* canvas = layout->findFunctionByName("CacaoCanvasMain");
		constexpr uint8_t PRE3D = (1 << 0);
		constexpr uint8_t POST3D = (1 << 1);
		constexpr uint8_t CUSTOM3D = (1 << 2);
		constexpr uint8_t PRE2D = (1 << 3);
		constexpr uint8_t CUSTOM2D = (1 << 4);
		constexpr uint8_t SURFACE = (1 << 5);
		constexpr uint8_t CANVAS = (1 << 6);
		uint8_t mask = 0;
		if(pre3D) mask |= PRE3D;
		if(post3D) mask |= POST3D;
		if(custom3D) mask |= CUSTOM3D;
		if(pre2D) mask |= PRE2D;
		if(custom2D) mask |= CUSTOM2D;
		if(surface) mask |= SURFACE;
		if(canvas) mask |= CANVAS;

		//Define validation functions to avoid code duplication
		const auto validate = [&](slang::FunctionReflection* rfl, const std::string& fn, const std::string& inType, const std::string& outType) {
			CheckException(rfl->getReturnType()->getScalarType() == slang::TypeReflection::Void, "Shader validation error: " + fn + " function return type is incorrect!\nHint: Function must return void.");
			CheckException(rfl->getParameterCount() == 2, "Shader validation error: " + fn + " function takes wrong parameter count!\n"
																							 "Hint: Function must take exactly two (2) arguments.");
			slang::VariableReflection* p1 = rfl->getParameterByIndex(0);
			CheckException(p1->findModifier(slang::Modifier::ID::Const) != nullptr, "Shader validation error: " + fn + " function parameter 1 is malformed!\nHint: Parameter 1 must be const.");
			ComPtr<slang::IBlob> fullP1TypeName;
			p1->getType()->getFullName(fullP1TypeName.writeRef());
			CheckException(!p1->getType()->isArray() && fullP1TypeName && std::string((const char*)fullP1TypeName->getBufferPointer(), fullP1TypeName->getBufferSize()).compare("Cacao." + inType) == 0,
				"Shader validation error: " + fn + " function parameter 1 is malformed!\nHint: Parameter 1 must be of type Cacao::" + inType + ".");
			slang::VariableReflection* p2 = rfl->getParameterByIndex(1);
			CheckException(p2->findModifier(slang::Modifier::ID::InOut) != nullptr, "Shader validation error: " + fn + " function parameter 2 is malformed!\nHint: Parameter 2 must be const.");
			ComPtr<slang::IBlob> fullP2TypeName;
			p2->getType()->getFullName(fullP2TypeName.writeRef());
			CheckException(!p2->getType()->isArray() && fullP2TypeName && std::string((const char*)fullP2TypeName->getBufferPointer(), fullP2TypeName->getBufferSize()).compare("Cacao." + outType) == 0,
				"Shader validation error: " + fn + " function parameter 2 is malformed!\nHint: Parameter 2 must be of type Cacao::" + outType + ".");
		};
		const auto canvasValidate = [canvas]() {
			CheckException(!canvas->getReturnType()->isArray() && canvas->getReturnType()->getScalarType() == slang::TypeReflection::Float32 && canvas->getReturnType()->getKind() == slang::TypeReflection::Kind::Vector &&
							   canvas->getReturnType()->getFieldCount() == 4,
				"Shader validation error: canvas function return type is incorrect!\n"
				"Hint: Canvas function must return a float4.");
			CheckException(canvas->getParameterCount() == 1, "Shader validation error: canvas function takes wrong parameter count!\n"
															 "Hint: Canvas function must take exactly one (1) argument.");
			slang::VariableReflection* cp1 = canvas->getParameterByIndex(0);
			CheckException(cp1->findModifier(slang::Modifier::ID::Const) != nullptr, "Shader validation error: canvas function parameter 1 is malformed!\n"
																					 "Hint: Canvas function parameter 1 must be const.");
			ComPtr<slang::IBlob> fullCP1TypeName;
			cp1->getType()->getFullName(fullCP1TypeName.writeRef());
			CheckException(!cp1->getType()->isArray() && fullCP1TypeName && std::string((const char*)fullCP1TypeName->getBufferPointer(), fullCP1TypeName->getBufferSize()).compare("Cacao.CanvasInput") == 0,
				"Shader validation error: canvas function parameter 1 is malformed!\n"
				"Hint: Canvas function parameter 1 must be of type Cacao::CanvasInput.");
		};

		//Validate!
		switch(mask) {
			case PRE3D | SURFACE: {
				try {
					validate(surface, "surface", "SurfaceInput", "SurfaceSample");
					validate(pre3D, "vertex preprocessing", "VSIn3D", "VSIn3D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Geometry3D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly;
				break;
			}
			case POST3D | SURFACE: {
				try {
					validate(surface, "surface", "SurfaceInput", "SurfaceSample");
					validate(post3D, "vertex postprocessing", "VSOut3D", "VSOut3D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Geometry3D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::PostprocessOnly;
				break;
			}
			case PRE3D | POST3D | SURFACE: {
				try {
					validate(surface, "surface", "SurfaceInput", "SurfaceSample");
					validate(pre3D, "vertex preprocessing", "VSIn3D", "VSIn3D");
					validate(post3D, "vertex postprocessing", "VSOut3D", "VSOut3D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Geometry3D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::PreprocessPostprocess;
				break;
			}
			case CUSTOM3D | SURFACE: {
				try {
					validate(surface, "surface", "SurfaceInput", "SurfaceSample");
					validate(custom3D, "custom vertex processing", "VSIn3D", "VSOut3D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Geometry3D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::Custom;
				break;
			}
			case SURFACE: {
				try {
					validate(surface, "surface", "SurfaceInput", "SurfaceSample");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Geometry3D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::NoProcess;
				break;
			}
			case PRE2D | CANVAS: {
				try {
					canvasValidate();
					validate(pre2D, "vertex preprocessing", "VSIn2D", "VSIn2D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Canvas2D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::PreprocessOnly;
				break;
			}
			case CUSTOM2D | CANVAS: {
				try {
					canvasValidate();
					validate(custom2D, "custom vertex processing", "VSIn2D", "VSOut2D");
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Canvas2D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::Custom;
				break;
			}
			case CANVAS: {
				try {
					canvasValidate();
				} catch(const std::exception& e) {
					CompileCheck(false, e.what());
				}
				shader.descriptor.domain = libcacaoasset::Shader::Descriptor::Domain::Canvas2D;
				shader.descriptor.mode = libcacaoasset::Shader::Descriptor::VertexMode::NoProcess;
				break;
			}
			default:
				CompileCheck(false, "Shader validation error: shader functions do not match any of the allowed configurations!");
		}
	}

	//Generate descriptor (part of validation because we need to check its validity)
	slang::TypeReflection* paramsType = layout->findTypeByName("MaterialParameters");
	if(paramsType) {
		for(unsigned int i = 0; i < paramsType->getFieldCount(); ++i) {
			//Setup Slang vars
			slang::VariableReflection* field = paramsType->getFieldByIndex(i);
			slang::TypeReflection* ftype = field->getType();
			ComPtr<slang::IBlob> typeNameBlob;
			ftype->getFullName(typeNameBlob.writeRef());
			std::string typeName = (typeNameBlob ? std::string((const char*)typeNameBlob->getBufferPointer(), typeNameBlob->getBufferSize()) : "unknown");

			//Validation per-type
			switch(ftype->getKind()) {
				case slang::TypeReflection::Kind::Scalar:
					//Checks
					CompileCheck(ftype->getScalarType() == slang::TypeReflection::ScalarType::Int32 || ftype->getScalarType() == slang::TypeReflection::ScalarType::UInt32 ||
									 ftype->getScalarType() == slang::TypeReflection::ScalarType::Float32 || ftype->getScalarType() == slang::TypeReflection::ScalarType::Bool,
						std::format("Shader validation error: material parameters block contains scalar of invalid type!\nHint: valid scalar types are bool, int, uint, and float.\nInvalid field: \"{}\" of type \"{}\".",
							field->getName(), typeName));

					//Parameter descriptor
					{
						libcacaoasset::Shader::Descriptor::UniformParameter uparam;
						uparam.name = field->getName();
						std::size_t off = layout->getTypeLayout(paramsType)->getFieldByIndex(i)->getOffset();
						CompileCheck(off != SLANG_UNKNOWN_SIZE, std::format("Shader validation error: material parameters block contains field with uncomputable offset!\nInvalid field: \"{}\" of type \"{}\".",
																	field->getName(), typeName));
						CompileCheck(off <= UINT32_MAX, std::format("Shader validation error: material parameters block is too large!\nError occured at field: \"{}\".",
															field->getName()));
						uparam.bufferOffset = off;
						switch(ftype->getScalarType()) {
							case slang::TypeReflection::ScalarType::Int32:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int;
								break;
							case slang::TypeReflection::ScalarType::UInt32:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt;
								break;
							case slang::TypeReflection::ScalarType::Float32:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float;
								break;
							case slang::TypeReflection::ScalarType::Bool:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Bool;
								break;
							default: break;
						}
						shader.descriptor.uniformParams.push_back(std::move(uparam));
					}
					break;
				case slang::TypeReflection::Kind::Vector: {
					//Checks
					CompileCheck(ftype->getScalarType() == slang::TypeReflection::ScalarType::Int32 || ftype->getScalarType() == slang::TypeReflection::ScalarType::UInt32 || ftype->getScalarType() == slang::TypeReflection::ScalarType::Float32,
						std::format("Shader validation error: material parameters block contains vector of invalid scalar type!\nHint: vectors may only be of int, uint, or float.\nInvalid field: \"{}\" of type \"{}\".",
							field->getName(), typeName));
					CompileCheck(ftype->getColumnCount() >= 2 && ftype->getColumnCount() <= 4, std::format("Shader validation error: material parameters block contains vector of invalid dimensions!\n"
																										   "Hint: valid vector dimensions are 2, 3, and 4.\nInvalid field: \"{}\" of type \"{}\".",
																								   field->getName(), typeName));

					//Parameter descriptor
					{
						libcacaoasset::Shader::Descriptor::UniformParameter uparam;
						uparam.name = field->getName();
						std::size_t off = layout->getTypeLayout(paramsType)->getFieldByIndex(i)->getOffset();
						CompileCheck(off != SLANG_UNKNOWN_SIZE, std::format("Shader validation error: material parameters block contains field with uncomputable offset!\nInvalid field: \"{}\" of type \"{}\".",
																	field->getName(), typeName));
						CompileCheck(off <= UINT32_MAX, std::format("Shader validation error: material parameters block is too large!\nError occured at field: \"{}\".",
															field->getName()));
						uparam.bufferOffset = off;
						switch(ftype->getScalarType()) {
							case slang::TypeReflection::ScalarType::Int32:
								switch(ftype->getColumnCount()) {
									case 2:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int2;
										break;
									case 3:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int3;
										break;
									case 4:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Int4;
										break;
									default: break;
								}
								break;
							case slang::TypeReflection::ScalarType::UInt32:
								switch(ftype->getColumnCount()) {
									case 2:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt2;
										break;
									case 3:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt3;
										break;
									case 4:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::UInt4;
										break;
									default: break;
								}
								break;
							case slang::TypeReflection::ScalarType::Float32:
								switch(ftype->getColumnCount()) {
									case 2:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2;
										break;
									case 3:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3;
										break;
									case 4:
										uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4;
										break;
									default: break;
								}
								break;
							default: break;
						}
						shader.descriptor.uniformParams.push_back(std::move(uparam));
					}
					break;
				}
				case slang::TypeReflection::Kind::Matrix:
					//Checks
					CompileCheck(ftype->getScalarType() == slang::TypeReflection::ScalarType::Float32,
						std::format("Shader validation error: material parameters block contains matrix of invalid scalar type!\nHint: all matrices must have scalar type float.\nInvalid field: \"{}\" of type \"{}\".",
							field->getName(), typeName));
					CompileCheck(ftype->getColumnCount() == ftype->getRowCount(), std::format("Shader validation error: material parameters block contains matrix of invalid dimensions!\n"
																							  "Hint: all matrices must have equal row and column counts (e.g. 3x3).\nInvalid field: \"{}\" of type \"{}\".",
																					  field->getName(), typeName));
					CompileCheck(ftype->getColumnCount() >= 2 && ftype->getColumnCount() <= 4, std::format("Shader validation error: material parameters block contains matrix of invalid dimensions!\n"
																										   "Hint: valid matrix dimensions are 2x2, 3x3, and 4x4.\nInvalid field: \"{}\" of type \"{}\".",
																								   field->getName(), typeName));

					//Parameter descriptor
					{
						libcacaoasset::Shader::Descriptor::UniformParameter uparam;
						uparam.name = field->getName();
						std::size_t off = layout->getTypeLayout(paramsType)->getFieldByIndex(i)->getOffset();
						CompileCheck(off != SLANG_UNKNOWN_SIZE, std::format("Shader validation error: material parameters block contains field with uncomputable offset!\nInvalid field: \"{}\" of type \"{}\".",
																	field->getName(), typeName));
						CompileCheck(off <= UINT32_MAX, std::format("Shader validation error: material parameters block is too large!\nError occured at field: \"{}\".",
															field->getName()));
						uparam.bufferOffset = off;
						switch(ftype->getColumnCount()) {
							case 2:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float2x2;
								break;
							case 3:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float3x3;
								break;
							case 4:
								uparam.type = libcacaoasset::Shader::Descriptor::UniformParameter::DataType::Float4x4;
								break;
							default: break;
						}
						shader.descriptor.uniformParams.push_back(std::move(uparam));
					}
					break;
				case slang::TypeReflection::Kind::Resource:
					//Checks
					CompileCheck(ftype->getResourceShape() == (SLANG_TEXTURE_2D | SLANG_TEXTURE_COMBINED_FLAG) || ftype->getResourceShape() == (SLANG_TEXTURE_CUBE | SLANG_TEXTURE_COMBINED_FLAG),
						std::format("Shader validation error: material parameters block contains resource of invalid type!\n"
									"Hint: valid resource types are Sampler2D and SamplerCube (combined 2D and cubemap image samplers).\nInvalid field: \"{}\" of type \"{}\".",
							field->getName(), typeName));
					CompileCheck(ftype->getResourceAccess() == SLANG_RESOURCE_ACCESS_READ, std::format("Shader validation error: material parameters block contains invalid texture (bad resource access)!\n"
																									   "Hint: Resource access must be read-only.\nInvalid field: \"{}\" of type \"{}\".",
																							   field->getName(), typeName));
					CompileCheck(ftype->getResourceResultType()->getKind() == slang::TypeReflection::Kind::Vector && ftype->getResourceResultType()->getScalarType() == slang::TypeReflection::ScalarType::Float32 &&
									 ftype->getResourceResultType()->getColumnCount() == 4,
						std::format("Shader validation error: material parameters block contains invalid texture (bad underlying type)!\nHint: Texture underlying type must be float4 "
									"(this is the default for texture types without specialization).\nInvalid field: \"{}\" of type \"{}\".",
							field->getName(), typeName));

					//Parameter descriptor
					{
						libcacaoasset::Shader::Descriptor::TextureParameter tparam;
						tparam.name = field->getName();
						tparam.isCubemap = (ftype->getResourceShape() & SLANG_TEXTURE_CUBE) > 0;
						std::size_t binding = layout->getTypeLayout(paramsType)->getFieldByIndex(i)->getBindingIndex();
						CompileCheck(binding != SLANG_UNKNOWN_SIZE && binding <= UINT32_MAX, std::format("Shader validation error: Slang assigned invalid binding for user material parameter!\nThis is not a problem with your shader, it is a BUG and should be reported.\n"
																										 "Error occured at field: \"{}\".",
																								 field->getName()));
						tparam.binding = binding;
						shader.descriptor.texParams.push_back(std::move(tparam));
					}
					break;
				default:
					CompileCheck(false, std::format("Shader validation error: material parameters block contains disallowed type!\nInvalid field: \"{}\" of type \"{}\".", field->getName(), typeName));
			}
		}
		if(!shader.descriptor.uniformParams.empty()) {
			for(libcacaoasset::Shader::Descriptor::TextureParameter& tparam : shader.descriptor.texParams) {
				++(tparam.binding);
			}
		}
	}
	CVLOG("Done.")

	//Write shader
	CVLOG_NONL("\tWriting output file " << out << "... ");
	std::ofstream outStream(out, std::ios::binary);
	CompileCheck(outStream.is_open(), "Failed to open output file!");
	libcacaoasset::EncodeShader(shader, &outStream);
	CVLOG("Done.");

	return {true, ""};
}