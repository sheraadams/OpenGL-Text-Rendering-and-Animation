# OpenGL Text Tendering and Animation

## About the Project

This program is an animation program designed to allow an interactive 3D environment. The text is rendered onto a 2D plane in the environment to add to the experience and deliver the message in a visually creative and interesting way. 


<div style="text-align: center;">
  <p><strong>Watch the video <a href="https://www.linkedin.com/feed/update/urn:li:activity:7011477997652238336/" target="_blank">here</a>.</strong></p>
</div>

<p align="center">
  <img width="" height="" src="https://user-images.githubusercontent.com/110789514/209887682-eebc1c47-1d9c-4ed8-b21e-01d5a1e30858.png">
</p>

## Getting Started

1. Clone this repo. Settings are Microsoft Visual Studio x64. 

2. Modify solution properties, (all solutions) edit debug > VCC++ > add includes add libraries.

3. Go to linker and add additional libraries of config, dlls, bin, resources.
go to properties > linker > input and add glfw3.lib, opengl32.lib, assimp.lib, and freetype.lib.

4. Go to project add all existing items to the project.

5. if filestystem.h warnings: 
after class FileSystem {
enter:
  public:
  #pragma warning(disable: 4996)

6. if using stb_image, add the following define below all includes: 
#define STB_IMAGE_IMPLEMENTATION

7. (optional) Add additional helpful macros. See https://github.com/nothings/stb/blob/master/stb_image.h

8. Set the C++ language properties and add most recent C++ version to use the 2020 libraries. 

## References

Credit to LearnOpenGL for shaders, camera, glfw functions and text rendering functions. 

https://github.com/JoeyDeVries/LearnOpenGL/blob/master/LICENSE.md

Learn OpenGL, extensive tutorial resource for learning Modern OpenGL. (n.d.-c). https://learnopengl.com/

<div style="text-align: center;">
  <p><strong>Proudly crafted with ❤️ by <a href="https://github.com/sheraadams" target="_blank">Shera Adams</a>.</strong></p>
</div>
