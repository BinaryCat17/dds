project('dds', 'cpp',
    version : '1.0',
    default_options : ['cpp_std=c++17', 'c_std=c11', 'warning_level=3'])

cmake = import('cmake')

# deps --------------------------------------------------------------------------------------------
cista = dependency('cista', required : false)
if not cista.found()
      cista = cmake.subproject('cista').dependency('cista')
endif

threads = dependency('threads')

# project -----------------------------------------------------------------------------------------

deps = [cista]
inc = include_directories('src')

lib = static_library('dds',
    'src/dds/data/instance.cpp',
    'src/dds/data/column.cpp',
    'src/dds/data/type.cpp',
    'src/dds/data/table.cpp',
    'src/dds/data/allocator.cpp',
    'src/dds/data/components.cpp',
    'src/dds/dds.cpp',
    install : true,
    dependencies : deps,
    include_directories : 'src')

dds_dep = declare_dependency(link_with : lib, include_directories : inc, dependencies : deps)

# tests -------------------------------------------------------------------------------------------

testApp = executable('testApp', 'app/testApp.cpp', dependencies : dds_dep)
