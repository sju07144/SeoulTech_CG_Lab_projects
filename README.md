# Image-based Deferred Renderer for IBL
## Overview
This project is a study of Image-based Deferred Rendering and aims to compare the images of forward rendering and deferred rendering under set conditions to explore the differences between them and reduce the differences as much as possible. The method is to output various scene parameters (albedo color, normal vector, etc.) used when performing forward rendering as images and use such images as texture maps to perform deferred rendering. For lighting techniques, image-based lighting is used by default, but you can also use Phong shading or general Physically-based Rendering techniques. When using PBR and IBL, you will also use metallic, roughness, and ao maps. In addition, in this project, we will calculate and output the error between the image output by performing forward rendering and the image output by performing image-based deferred rendering. The error will mainly use RMSE.
## Tech spec
### Hardware spec
- OS: Windows 10
- CPU: Intel® Core™ i5-8250U Processor 1.6GHz -> AMD Ryzen™ 5 7600X Desktop Processor 3.8GHz
- RAM: 12GB -> 32GB
- GPU: Intel® UHD Graphics 620 -> NVIDIA GeForce RTX 3060 
### Deferred Renderer
- Language: ISO C++ 14 standard
- Development Environment: Visual Studio 2019 -> Visual Studio 2022
### Loss Calculator
- OS: Windows 10
- Language: Python 3.9 -> 3.10
- Development Environment: Visual Studio Code
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
- numpy 1.26.2
- matplotlib 3.3.4 -> 3.8.1
- pandas 2.1.3
- OpenCV 4.5.3 -> 4.9.0
- skimage 0.22.0
- sqlite3 3.33.0 -> 3.35.5
## Notes
.glb loader and renderer for IBL forward rendering
- https://github.com/Hoyeon9/glbLoadCapture
Almost of the codes for the rendering refer to 'Learn OpenGL'.
- [IBL rendering - Learn OpenGL](https://learnopengl.com/PBR/IBL/Diffuse-irradiance)
