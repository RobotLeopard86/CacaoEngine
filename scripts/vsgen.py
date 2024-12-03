import pathlib
import json
import os
import re

def parse_compile_commands(file_path):
	# Load the compile_commands.json file
	with open(file_path, 'r') as f:
		compile_commands = json.load(f)
	
	include_paths = set()  # Use a set to avoid duplicates
	script_dir = os.getcwd()  # Directory where the script is run

	# Iterate through each entry in the JSON
	for entry in compile_commands:
		# Extract the compiler command or arguments
		command = entry.get('command', '') or ' '.join(entry.get('arguments', []))
		
		# Match include paths (-Ipath or /Ipath)
		matches = re.findall(r'(?:-I|/I)(\S+)', command)
		
		for match in matches:
			# Convert relative paths to absolute paths based on the directory of the current entry
			base_dir = entry.get('directory', '.')
			abs_path = os.path.normpath(os.path.join(base_dir, match))

			ins_path = abs_path
			
			# Make the path relative to the script directory if it is
			if abs_path.startswith(os.path.normpath(script_dir)):
				ins_path = os.path.relpath(abs_path, script_dir)
			
			# Format as $(ProjectDir)path;
			include_paths.add(f'$(ProjectDir){ins_path};')

	return ''.join(include_paths)

# Solution file
print("Writing solution file...")
with open('CacaoEngine.sln', 'w') as sln:
	sln.writelines([
		'Microsoft Visual Studio Solution File, Format Version 12.00\n',
		'# Visual Studio Version 17\n',
		'VisualStudioVersion = 17.10.35013.160\n',
		'MinimumVisualStudioVersion = 10.0.40219.1\n',
		'Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "Cacao", "Cacao.vcxproj", "{A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}"\n',
		'EndProject\n',
		'Global\n',
		'    GlobalSection(SolutionConfigurationPlatforms) = preSolution\n',
		'        Debug|x64 = Debug|x64\n',
		'        Release|x64 = Release|x64\n',
		'    EndGlobalSection\n',
		'    GlobalSection(ProjectConfigurationPlatforms) = postSolution\n',
		'        {A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}.Debug|x64.ActiveCfg = Debug|x64\n',
		'        {A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}.Debug|x64.Build.0 = Debug|x64\n',
		'        {A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}.Release|x64.ActiveCfg = Release|x64\n',
		'        {A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}.Release|x64.Build.0 = Release|x64\n',
		'    EndGlobalSection\n',
		'    GlobalSection(SolutionProperties) = preSolution\n',
		'        HideSolutionNode = FALSE\n',
		'    EndGlobalSection\n',
		'    GlobalSection(ExtensibilityGlobals) = postSolution\n',
		'        SolutionGuid = {21E313CF-3126-4C3D-A989-CFF684865B94}\n',
		'    EndGlobalSection\n',
		'EndGlobal\n'
	])
	
# Scan for C++ and header files
print("Scanning for code files...")
inc_lines = []
src_lines = []
for dir in ['backends', 'cacao', 'playground']:
	directory = pathlib.Path(dir)
	for file_path in directory.rglob('*'):
		if file_path.suffix == '.hpp':
			backslash_path = file_path.as_posix().replace('/', '\\')
			formatted_line = f'<ClInclude Include="{backslash_path}" />\n'
			inc_lines.append(formatted_line)
		elif file_path.suffix == '.cpp':
			backslash_path = file_path.as_posix().replace('/', '\\')
			formatted_line = f'<ClCompile Include="{backslash_path}" />\n'
			src_lines.append(formatted_line)

# Fetch include directories
print("Fetching include directory list...")
dbg_includes = parse_compile_commands("Debug/compile_commands.json")
rel_includes = parse_compile_commands("Release/compile_commands.json")

# VCXPROJ file
print("Writing project file...")
with open('Cacao.vcxproj', 'w') as vcx:
	all_lines = [
		[
		'<?xml version="1.0" encoding="utf-8"?>\n',
		'<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">\n',
		'  <ItemGroup Label="ProjectConfigurations">\n',
		'    <ProjectConfiguration Include="Debug|x64">\n',
		'      <Configuration>Debug</Configuration>\n',
		'      <Platform>x64</Platform>\n',
		'    </ProjectConfiguration>\n',
		'    <ProjectConfiguration Include="Release|x64">\n',
		'      <Configuration>Release</Configuration>\n',
		'      <Platform>x64</Platform>\n',
		'    </ProjectConfiguration>\n',
		'  </ItemGroup>\n',
		'  <PropertyGroup Label="Globals">\n',
		'    <VCProjectVersion>17.0</VCProjectVersion>\n',
		'    <ProjectGuid>{A3DB4047-E0C5-4756-B9BB-6CBFCA3B593C}</ProjectGuid>\n',
		'    <Keyword>Win32Proj</Keyword>\n',
		'  </PropertyGroup>\n',
		'  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />\n',
		'  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|x64\'" Label="Configuration">\n',
		'    <ConfigurationType>Makefile</ConfigurationType>\n',
		'    <UseDebugLibraries>true</UseDebugLibraries>\n',
		'    <PlatformToolset>v143</PlatformToolset>\n',
		'  </PropertyGroup>\n',
		'  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'" Label="Configuration">\n',
		'    <ConfigurationType>Makefile</ConfigurationType>\n',
		'    <UseDebugLibraries>false</UseDebugLibraries>\n',
		'    <PlatformToolset>v143</PlatformToolset>\n',
		'  </PropertyGroup>\n',
		'  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />\n',
		'  <ImportGroup Label="ExtensionSettings">\n',
		'  </ImportGroup>\n',
		'  <ImportGroup Label="Shared">\n',
		'  </ImportGroup>\n',
		'  <ImportGroup Label="PropertySheets" Condition="\'$(Configuration)|$(Platform)\'==\'Debug|x64\'">\n',
		'    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />\n',
		'  </ImportGroup>\n',
		'  <ImportGroup Label="PropertySheets" Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'">\n',
		'    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />\n',
		'  </ImportGroup>\n',
		'  <PropertyGroup Label="UserMacros" />\n',
		'  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|x64\'">\n',
		'    <NMakeBuildCommandLine>meson compile -C $(Configuration) bundle</NMakeBuildCommandLine>\n',
		'    <NMakeOutput>$(Configuration)\\playground-bundled\\cacaoengine.exe</NMakeOutput>\n',
		'    <NMakeCleanCommandLine>meson compile -C $(Configuration) --clean</NMakeCleanCommandLine>\n',
		'    <NMakeReBuildCommandLine>meson compile -C $(Configuration) --clean &amp;&amp; meson compile -C $(Configuration) bundle</NMakeReBuildCommandLine>\n',
		'    <NMakePreprocessorDefinitions>_DEBUG;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>\n',
		f'    <IncludePath>$(VC_IncludePath);$(WindowsSDK_IncludePath);{dbg_includes}</IncludePath>\n',
		'  </PropertyGroup>\n',
		'  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'">\n',
		'    <NMakeBuildCommandLine>meson compile -C $(Configuration) playground</NMakeBuildCommandLine>\n',
		'    <NMakeOutput>$(Configuration)\\playground-bundled\\cacaoengine.exe</NMakeOutput>\n',
		'    <NMakeCleanCommandLine>meson compile -C $(Configuration) --clean</NMakeCleanCommandLine>\n',
		'    <NMakeReBuildCommandLine>meson compile -C $(Configuration) --clean &amp;&amp; meson compile -C $(Configuration) bundle</NMakeReBuildCommandLine>\n',
		'    <NMakePreprocessorDefinitions>_DEBUG;$(NMakePreprocessorDefinitions)</NMakePreprocessorDefinitions>\n',
		f'    <IncludePath>$(VC_IncludePath);$(WindowsSDK_IncludePath);{rel_includes}</IncludePath>\n',
		'  </PropertyGroup>\n',
		'  <ItemDefinitionGroup>\n',
		'  </ItemDefinitionGroup>\n',
		'  <ItemGroup>\n',
		'    <None Include=".clang-format" />\n',
		'    <None Include=".clang-format-ignore" />\n',
		'    <None Include=".gitignore" />\n',
		'    <None Include=".gitmodules" />\n',
		'    <None Include="BACKENDS.md" />\n',
		'    <None Include="BUILD.md" />\n',
		'    <None Include="default_native.ini" />\n',
		'    <None Include="LICENSE" />\n',
		'    <None Include="meson.build" />\n',
		'    <None Include="meson.options" />\n',
		'    <None Include="playground\\src\\shaders\\ico.frag" />\n',
		'    <None Include="playground\\src\\shaders\\ico.vert" />\n',
		'    <None Include="playground\\src\\shaders\\prism.frag" />\n',
		'    <None Include="playground\\src\\shaders\\prism.vert" />\n',
		'    <None Include="README.md" />\n',
		'  </ItemGroup>\n',
		'  <ItemGroup>\n',
		'    <Image Include="cacaologo.png" />\n',
		'  </ItemGroup>\n',
		'  <ItemGroup>\n'],
		inc_lines,
		['  </ItemGroup>\n',
		'  <ItemGroup>\n'],
		src_lines,
		['  </ItemGroup>\n',
		'  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />\n',
		'  <ImportGroup Label="ExtensionTargets">\n',
		'  </ImportGroup>\n',
		'</Project>\n']
	]
	flat_lines = []
	for linegroup in all_lines:
		flat_lines.extend(linegroup)
	
	vcx.writelines(flat_lines)

print('Complete.')