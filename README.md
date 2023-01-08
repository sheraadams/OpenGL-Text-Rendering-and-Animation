# Documentation

This program is a visualization/ simulation program designed to allow an interactive 3D environment. 

<p align="center">
  <img width="" height="" src="https://user-images.githubusercontent.com/110789514/209887682-eebc1c47-1d9c-4ed8-b21e-01d5a1e30858.png">
</p>


## Instructions

1. clone. Settings are MS Visual Studio x64. 

2. Modify solution properties, all solutions debug > VCC++.
add includes, add libraries

3. go to linker link additional libraries of config, dlls, bin, resources.
go to linker > input in properties add 
glfw3.lib, opengl32.lib, assimp.lib, freetype.lib

4. go to project add all existing items if missing items from below:
glad.c, stb_image.cpp, .pdbs, source, etc without duplicating source or adding solutions or other msvs files already detected. 

5. if filestystem.h warnings: 
after class FileSystem {
enter:
  public:
  #pragma warning(disable: 4996)

6. if using stb_image, add the following define below all includes: 
#define STB_IMAGE_IMPLEMENTATION
additional helpful macros:
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_MALLOC
#define STBI_REALLOC 
#define STBI_FREE
#define STBI_NO_FAILURE_STRINGS
more info: see https://github.com/nothings/stb/blob/master/stb_image.h
if stb errors check for duplicate stb.cpp. 

7. using freetype, be sure to add freetype.lib, but also go to C++ language properties and add most recent c++ version to use the 2020 libraries. 

## Attribution

Joey Devries and LearnOpenGL as much of this repository is inspired by this site. 

## References

Learn OpenGL, extensive tutorial resource for learning Modern OpenGL. (n.d.-c). https://learnopengl.com/
