# Get mode
formats_mode = get_option('formats_mode')

# Define source lists
packed_src = []
unpacked_src = []

# Calculate final sources list
final_formats_src = []
if formats_mode == 'packed' or formats_mode == 'both'
	final_formats_src += packed_src
endif
if formats_mode == 'unpacked' or formats_mode == 'both'
	final_formats_src += unpacked_src
endif

formats_lib = static_library('cacaoformats', sources: final_formats_src, include_directories: 'include', pic: true)

formats_dep = declare_dependency(include_directories: 'include', link_with: formats_lib)