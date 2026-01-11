project('unordered_dense', 'cpp',
    version: '4.5.0',
    license: 'MIT',
    default_options : [
        'cpp_std=c++17', 
        'warning_level=3',
        'werror=true', 
        'b_ndebug=true', # otherwise absl is really slow!
    ])

incdir = include_directories('include')
unordered_dense_dep = declare_dependency(include_directories: incdir)