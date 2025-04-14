# Material compiler (basically just a libcacaoformats wrapper)
cubetool = executable('ce-cubetool', sources: [
    'main.cpp',
    'create.cpp',
    'extract.cpp'
], dependencies: [
    formats_dep,
    cli11_dep,
    spinners_dep
], cpp_args: ['-DCACAO_VER="' + meson.project_version() + '"', '-DTOOL_VER="1.0.0"'], include_directories: '..')