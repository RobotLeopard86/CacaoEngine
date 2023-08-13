MODULES := imgui_core glad_gl3 glfw_x imgui_gl3 imgui_glfw citrus_core citrus_backend_lnx_glfwx_gl3

.PHONY: default $(MODULES)

default:
	@echo "Citrus Engine GNU Make Module Builder"
	@echo "========================================="
	@echo ""
	@echo "Do not run directly. This should only be invoked by the top-level Makefile's 'build-module' command."

imgui_core:
	@echo "Building module 'imgui_core'..."
	@echo ""
	@${MAKE} --no-print-directory -f imgui_core.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'imgui_core'."

imgui_glfw: imgui_core
	@echo "Building module 'imgui_glfw'..."
	@echo ""
	@${MAKE} --no-print-directory -f imgui_glfw.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'imgui_glfw'."

imgui_gl3: imgui_core glad_gl3
	@echo "Building module 'imgui_gl3'..."
	@echo ""
	@${MAKE} --no-print-directory -f imgui_glfw.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'imgui_gl3'."

glfw_x:
	@echo "Building module 'glfw_x'..."
	@echo ""
	@${MAKE} --no-print-directory -f glfw_x.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'glfw_x'."

glad_gl3:
	@echo "Building module 'glad_gl3'..."
	@echo ""
	@${MAKE} --no-print-directory -f glad_gl3.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'glad_gl3'."

citrus_core: imgui_core
	@echo "Building module 'citrus_core'..."
	@echo ""
	@${MAKE} --no-print-directory -f citrus_core.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'citrus_core'."

citrus_backend_lnx_glfwx_gl3: citrus_core glfw_x imgui_glfw imgui_gl3
	@echo "Building module 'citrus_core'..."
	@echo ""
	@${MAKE} --no-print-directory -f citrus_core.mk build config=$(config) c="$(c)" cpp="$(cpp)"
	@echo ""
	@echo "Done building module 'citrus_core'."