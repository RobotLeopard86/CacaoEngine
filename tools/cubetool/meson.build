# Material compiler (basically just a libcacaoformats wrapper)
cubetool = executable('ce-cubetool', sources: [
    'main.cpp',
    'create.cpp',
    'extract.cpp'
], dependencies: [
    formats_dep,
    cli11_dep
], cpp_args: '-DTOOL_VER="1.0.0"', include_directories: '..', install: true)