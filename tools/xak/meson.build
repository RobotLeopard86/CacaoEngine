# Asset pack tool
xak = executable('ce-xak', sources: [
    'main.cpp', 'create.cpp', 'delete.cpp', 'extract.cpp', 'list.cpp', 'merge.cpp'
], dependencies: [formats_dep, cli11_dep, spinners_dep, yaml_cpp], cpp_args: ['-DCACAO_VER="' + meson.project_version() + '"', '-DXAK_VER="1.0.0"'], include_directories: '..')