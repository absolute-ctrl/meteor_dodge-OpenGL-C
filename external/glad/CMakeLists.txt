# glad: carregador de funcoes OpenGL (gerado em glad.dav1d.de, gl 4.0 core).
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC include)

# glad.c e codigo gerado; nao queremos avisos dele poluindo o build.
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(glad PRIVATE -w)
endif()
