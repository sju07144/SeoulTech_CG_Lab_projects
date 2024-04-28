# Image-based Deferred Renderer for IBL
This project is a study of Image-based Deferred Rendering and aims to compare the images of forward rendering and deferred rendering under set conditions to explore the differences between them and reduce the differences as much as possible. The method is to output various scene parameters (albedo color, normal vector, etc.) used when performing forward rendering as images and use such images as texture maps to perform deferred rendering. For lighting techniques, image-based lighting is used by default, but you can also use Phong shading or general Physically-based Rendering techniques. When using PBR and IBL, you will also use metallic, roughness, and ao maps. In addition, in this project, we will calculate and output the error between the image output by performing forward rendering and the image output by performing image-based deferred rendering. The error will mainly use RMSE.

Translated with DeepL.com (free version)
## Tech spec
ISO C++ 14 standard
Python 3.9
## Dependencies
### Deferred Renderer
- [GLM](https://github.com/g-truc/glm): Calculations for OpenGL
- [GLFW](https://glfw.org/): Create window, context
- [GLAD](https://glad.dav1d.de/): Load functions
- [Assimp](https://assimp.org/): Load 3d models
- [OpenCV](https://opencv.org/releases/): Image processing
- [`stb_image.h`](https://github.com/nothings/stb): Load image files and proccess
- [Dear ImGui](https://github.com/ocornut/imgui): for GUI
### LossCalculator
- 
## Before Execution
- Check if all the links for the external libraries are set properly.
  - You might need .dll files for 'OpenCV' and 'Assimp'.
  - Rebuild in your environment if needed.
- Check string variables below for your paths.
  - modelsLoc
  - hdrLoc
  - savePath
  - ~~textureLoc~~
- Result images will be saved in the directories of your 'savePath'.
## Notes
.glb loader and renderer for IBL forward rendering
- https://github.com/Hoyeon9/glbLoadCapture
Almost of the codes for the rendering refer to 'Learn OpenGL'.
- [IBL rendering - Learn OpenGL](https://learnopengl.com/PBR/IBL/Diffuse-irradiance)
