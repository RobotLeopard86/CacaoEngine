ifndef config
  	config = debug
endif

ifndef verbose
	SILENT = @
endif

BACKENDS := OpenGL

api :=
correctedapi = 0

ifndef renderapi
	api = OpenGL
endif

# Is the rendering API OpenGL?
ifeq ($(renderapi),OpenGL)
    api = OpenGL
else
	correctedapi = 1
    api = OpenGL
endif

.PHONY: clean prebuild

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
	SHELLTYPE := msdos
endif

# Configurations
# #############################################

WINDRES = comp
INCLUDES += -Isrc -I../libs/spdlog/include -I../libs/glm -I../libs/glfw/include -I../libs/glad/include -I../libs/boost/include
FORCE_INCLUDE +=
ALL_CPPFLAGS += $(CPPFLAGS) -MD -MP $(DEFINES) $(INCLUDES)
ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
define PREBUILDCMDS
endef
define PRELINKCMDS
endef
define POSTBUILDCMDS
endef

ifeq ($(config),debug)
TARGETDIR = ../Build/CitrusEngine/Debug/x86_64-linux/Binaries
TARGET = $(TARGETDIR)/libCitrusEngine.a
TEST-TARGET = $(TARGETDIR)/tests/CitrusEngineTests
OBJDIR = ../Build/CitrusEngine/Debug/x86_64-linux/CompilerData
DEFINES += -DCE_PLATFORM_LINUX -DCE_ENV_DEBUG -DCE_ENABLEASSERTS -DGLFW_INCLUDE_NONE
ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -g -std=gnu17
ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m64 -g -std=gnu++20
LIBS +=-lGLFW -lGlad -lImGui -lGL -lX11 -lXrandr -lXi -lboost_test_exec_monitor -lboost_unit_test_framework -lboost_prg_exec_monitor
LDDEPS += ../Build/GLFW/Debug/x86_64-linux/Binaries/libGLFW.a ../Build/Glad/Debug/x86_64-linux/Binaries/libGlad.a ../Build/ImGui/Debug/x86_64-linux/Binaries/libImGui.a 
ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -m64 -L../Build/GLFW/Debug/x86_64-linux/Binaries -L../Build/Glad/Debug/x86_64-linux/Binaries -L../Build/ImGui/Debug/x86_64-linux/Binaries -L../libs/boost/lib -static

else ifeq ($(config),release)
TARGETDIR = ../Build/CitrusEngine/Release/x86_64-linux/Binaries
TARGET = $(TARGETDIR)/libCitrusEngine.a
TEST-TARGET = $(TARGETDIR)/tests/CitrusEngineTests
OBJDIR = ../Build/CitrusEngine/Release/x86_64-linux/CompilerData
DEFINES += -DCE_PLATFORM_LINUX -DCE_ENV_RELEASE -DGLFW_INCLUDE_NONE
ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -m64 -O2 -std=gnu17
ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -m64 -O2 -std=gnu++20
LIBS +=-lGLFW -lGlad -lImGui -lGL -lX11 -lXrandr -lXi
LDDEPS += ../Build/GLFW/Release/x86_64-linux/Binaries/libGLFW.a ../Build/Glad/Release/x86_64-linux/Binaries/libGlad.a ../Build/ImGui/Release/x86_64-linux/Binaries/libImGui.a
ALL_LDFLAGS += $(LDFLAGS) -L/usr/lib64 -m64 -s -L../Build/GLFW/Release/x86_64-linux/Binaries -L../Build/Glad/Release/x86_64-linux/Binaries -L../Build/ImGui/Release/x86_64-linux/Binaries

endif

# Per File Configurations
# #############################################


# File sets
# #############################################

GENERATED :=
OBJECTS :=

GENERATED += $(OBJDIR)/Entrypoint.o
OBJECTS += $(OBJDIR)/Entrypoint.o
GENERATED += $(OBJDIR)/Log.o
OBJECTS += $(OBJDIR)/Log.o
GENERATED += $(OBJDIR)/Assert.o
OBJECTS += $(OBJDIR)/Assert.o
GENERATED += $(OBJDIR)/CitrusClient.o
OBJECTS += $(OBJDIR)/CitrusClient.o
GENERATED += $(OBJDIR)/Window.o
OBJECTS += $(OBJDIR)/Window.o
GENERATED += $(OBJDIR)/Input.o
OBJECTS += $(OBJDIR)/Input.o
GENERATED += $(OBJDIR)/EventManager.o
OBJECTS += $(OBJDIR)/EventManager.o
GENERATED += $(OBJDIR)/Renderer.o
OBJECTS += $(OBJDIR)/Renderer.o
GENERATED += $(OBJDIR)/Utilities.o
OBJECTS += $(OBJDIR)/Utilities.o
GENERATED += $(OBJDIR)/NativeShader.o
OBJECTS += $(OBJDIR)/NativeShader.o
GENERATED += $(OBJDIR)/NativeRenderer.o
OBJECTS += $(OBJDIR)/NativeRenderer.o
GENERATED += $(OBJDIR)/NativeWindow.o
OBJECTS += $(OBJDIR)/NativeWindow.o
GENERATED += $(OBJDIR)/NativeUtilities.o
OBJECTS += $(OBJDIR)/NativeUtilities.o
GENERATED += $(OBJDIR)/NativeInput.o
OBJECTS += $(OBJDIR)/NativeInput.o

TESTOBJECTS :=
TESTGENERATED :=

TESTGENERATED += $(OBJDIR)/tests/TestEntrypoint.o
TESTOBJECTS += $(OBJDIR)/tests/TestEntrypoint.o


# Rules
# #############################################

all: $(TARGET)
	@:

test: $(TEST-TARGET)
	@$(TEST-TARGET)

$(TARGET): $(TARGETDIR) $(OBJDIR) $(OBJECTS) $(LDDEPS) | $(TARGETDIR)
	$(PRELINKCMDS)
	@echo Linking CitrusEngine...
	@$(SILENT) $(AR) -rcs ../Build/CitrusEngine/Debug/x86_64-linux/CompilerData/libCitrusEngineBase.a $(OBJECTS)
	@$(SILENT) echo -e "create ../Build/CitrusEngine/Debug/x86_64-linux/Binaries/libCitrusEngine.a\naddlib ../Build/CitrusEngine/Debug/x86_64-linux/CompilerData/libCitrusEngineBase.a\naddlib ../Build/Glad/Debug/x86_64-linux/Binaries/libGlad.a\naddlib ../Build/GLFW/Debug/x86_64-linux/Binaries/libGLFW.a\naddlib ../Build/ImGui/Debug/x86_64-linux/Binaries/libImGui.a\nsave\nend" | $(AR) -M
	$(POSTBUILDCMDS)

$(TEST-TARGET): $(TARGETDIR)/tests $(OBJDIR)/tests $(TESTOBJECTS) $(LDDEPS) | $(TARGETDIR)/tests
	$(PRELINKCMDS)
	@echo Linking test executable...
	@$(CXX) -o "$@" $(TESTOBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(TARGET)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)...
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)...
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

$(TARGETDIR)/tests:
	@echo Creating $(TARGETDIR)/tests...
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)/tests
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR)/tests)
endif

$(OBJDIR)/tests:
	@echo Creating $(OBJDIR)/tests...
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)/tests
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR)/tests)
endif

clean:
	@echo Cleaning CitrusEngine
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -f  $(TEST-TARGET)
	$(SILENT) rm -rf $(GENERATED)
	$(SILENT) rm -rf $(TESTGENERATED)
	$(SILENT) rm -rf $(OBJDIR)
	$(SILENT) rm -rf $(OBJDIR)/tests
	$(SILENT) rm -rf $(TARGETDIR)/tests
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(GENERATED)) del /s /q $(subst /,\\,$(GENERATED))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild: | $(OBJDIR)
	$(PREBUILDCMDS)

ifneq (,$(PCH))
$(OBJECTS): $(GCH) | $(PCH_PLACEHOLDER)
$(GCH): $(PCH) | prebuild
	@echo $(notdir $<)
	$(SILENT) $(CXX) -x c++-header $(ALL_CXXFLAGS) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
$(PCH_PLACEHOLDER): $(GCH) | $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) touch "$@"
else
	$(SILENT) echo $null >> "$@"
endif
else
$(OBJECTS): | prebuild
endif


# Test File Rules
# #############################################

$(OBJDIR)/tests/TestEntrypoint.o: tests/TestingEntrypoint.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"



# File Rules
# #############################################

$(OBJDIR)/Entrypoint.o: src/Core/Entrypoint.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Log.o: src/Core/Log.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/CitrusClient.o: src/Core/CitrusClient.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Assert.o: src/Core/Assert.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/EventManager.o: src/Events/EventManager.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Window.o: src/Graphics/Window.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Renderer.o: src/Graphics/Renderer.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Input.o: src/Utilities/Input.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/Utilities.o: src/Utilities/Utilities.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/NativeShader.o: src/$(api)/$(api)Shader.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/NativeRenderer.o: src/$(api)/$(api)Renderer.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/NativeWindow.o: src/Linux/LinuxWindow.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/NativeInput.o: src/Linux/LinuxInput.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

$(OBJDIR)/NativeUtilities.o: src/Linux/LinuxUtilities.cpp
	@echo "Building C++ file $(notdir $<)..."
	$(SILENT) $(CXX) $(ALL_CXXFLAGS) $(FORCE_INCLUDE) $(LIBS) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

-include $(OBJECTS:%.o=%.d)
ifneq (,$(PCH))
  -include $(PCH_PLACEHOLDER).d
endif