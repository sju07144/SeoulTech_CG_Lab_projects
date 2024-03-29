# SeoulTech_CG_Lab_projects
Image-based Deferred Renderer for IBL

## 개요
본 프로젝트에서는 Image-based Deferred Renderer를 구현한다. Forward rendering을 수행할 때 사용하는 각종 scene parameter(albedo color, normal 벡터 등)를 이미지로 출력하고 그러한 이미지를 texture map 형태로 이용하여 deferred rendering을 수행하는 프로젝트이다. 조명 기법으로는 기본적으로 image-based lighting을 사용하지만 Phong shading 또는 일반적인 Physically-based Rendering 기법도 사용할 수 있다. PBR과 IBL를 사용할 경우, metallic, roughness, ao map도 이용하게 된다. 그리고 pixtopix를 이용하여 생성한 texture map을 이용하여 Deferred Rendering을 수행하기도 한다. 또한, 본 프로젝트에서는 forward rendering을 수행하여 출력한 이미지와 image-based deferred rendering을 수행하여 출력한 이미지와의 오차(주로, RMSE)를 계산하고 출력한다.

## 개발환경
- 'C++20'(for renderer), 'Python 3.10'(for loss calculator), 'GLSL version 430'(for shader)
- 'Visual studio 2019'(for renderer), 'Visual studio code'(for loss calculator)
- Framework: OpenGL 4.3, OpenCV 4.8
- Library: GLFW, glad, assimp, glm, stb
- GUI: imgui (https://github.com/ocornut/imgui)

## 실행방법
솔루션 파일을 열어 실행을 해주면 윈도우 창을 하나 생성하며 이 윈도우 창에서 렌더링을 수행한다. 특정 카메라 벡터에서의 수행이 한 번 끝나면 렌더링된 이미지를 캡쳐하여 저장하게 되며 완료할 시, 커맨드 창에서 "Save Completed ***.png"를 출력한다.

## 각종 Directory 정보
- cores: 기본 geometry 생성(box, sphere, quad 등), camera 생성, mesh 초기화, model 파일 로딩, shader 빌드, texture 생성 등을 담당하는 코드를 담아둠.
- DeferredRenderer: DeferredRenderer의 하위폴더인 ImagebasedRenderer에 프로젝트 파일, main.cpp(main 함수), Renderer.h/Renderer.cpp가 담겨있음.
- Libraries: 필요한 라이브러리 파일이 담겨있음.(OpenGL, OpenCV, Assimp 등)
- LossCalculator: 오차 계산을 위한 파이썬 프로그램이 담겨있음.
- resources: shader 파일, texture 파일, model 파일, ground truth 이미지, 본 렌더러로 렌더링한 이미지를 담고있음.
- 언급하지 않은 나머지는 연습용

## 중요 사항
- Renderer.cpp에서 모델 디렉토리 이름을 검색할 때 C++20의 기능을 사용함. 프로젝트 설정에서 꼭 C++20으로 체크하고 빌드!!(그렇지 않을 경우 따로 boost 라이브러리 추가 필요)

### resources 하위 폴더 일부 설명
- compare: shader에서 계산한 각종 벡터값(Diffuse, Specular, BRDF, Fresnel 등)을 출력한 이미지를 모아놓은 폴더
- IBL_rendered_examples: ground truth 이미지 및 image-based rendered 이미지를 모아놓은 폴더
- models: 모델 파일을 모아놓은 폴더
- shaders: shader 파일을 모아놓은 폴더
- statistics: 통계 자료를 모아놓은 폴더
- textures: texture 파일을 모아놓은 폴더

### shaders 하위 파일 설명
- .vert: vertex shader, .frag: fragment shader
- brdf: brdf lookup table 생성 코드(for image-based light)
- cubemap: cubemap 렌더링 코드
- cubemapHDR: cubemap 렌더링 코드(HDR)
- equirectangularToCube: HDR 이미지를 cubemap으로 전환하는 코드(for image-based light)
- irradianceMap: Irradiance Map 생성 코드(for image-based light)
- pbr_deferred: image-based light 등 PBR deferred 렌더링을 수행하는 코드
- prefiltermap: Pre=filter map 생성 코드(for image-based light)
- 언급하지 않은 코드는 연습용

### LossCalculator 하위 파일 설명
- csv_test.py: view vector의 각도(theta, phi)에 따른 오차를 3차원 막대그래프로 보여줌.
- difference_image.py: ground truth 이미지와 image-based rendered 이미지의 contrast를 pixel-by-pixel로 계산하여 윈도우에 출력.
- difference_image2.py: shader에서 계산한 벡터값들을 출력한 ground truth 이미지와 image-based rendered 이미지의 contrast를 pixel-by-pixel로 계산하여 윈도우에 출력.
- loss_calculator.py: mse 오차를 계산하여 csv 파일화까지 수행하는 코드. mse 함수는 skimage 모듈에 있는 것을 사용하였고 background를 제외할 경우 custom 함수를 사용.
- mse_test.py: mse 오차를 정렬하여 최댓값, 최솟값을 확인하기 위한 코드
- mse_test2.py: mse_test와 기능은 똑같으나 실제 이미지가 아닌 shader에서 계산한 벡터값을 출력한 이미지
