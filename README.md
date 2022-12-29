# OpenGL-Text-Scene

Instructions:

1. clone. Settings are MS Visual Studio x64.

2. Modify solution properties, all solutions debug > VCC++.
add includes, add libraries

3. go to linker link additional libraries of config, dlls, bin, resources.
go to linker > input in properties add 
glfw3.lib, opengl32.lib, assimp.lib, freetype.lib

4. go to project add existing items if missing items from below:
glad.c, stb_image.cpp, .pdbs

5. if filestystem.h warnings: 
after class FileSystem {
enter:
  public:
  #pragma warning(disable: 4996)


6. if using stb_image type above stb include: 
#define STB_IMAGE_IMPLEMENTATION 
stb errors check for duplicate stb.cpp. If stb errors still, move #define STB_IMAGE_IMPLEMENTATION  below all includes.

7. using freetype, be sure to add freetype.lib, but also go to C++ language properties and add most recent c++ version to use the 2020 libraries. 

![2 (4)](https://user-images.githubusercontent.com/110789514/209887682-eebc1c47-1d9c-4ed8-b21e-01d5a1e30858.png)
