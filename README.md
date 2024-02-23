# SeoulTech_CG_Lab_projects
Image-based Lighting Deferred Renderer

## Directory Information
- cores: 기본 geometry 생성(box, sphere, quad 등), camera 생성, mesh 초기화, model 파일 로딩, shader 빌드, texture 생성 등을 담당하는 코드를 담아둠.
- DeferredRenderer: DeferredRenderer의 하위폴더인 ImagebasedRenderer에 프로젝트 파일, main.cpp(main 함수), Renderer.h/Renderer.cpp가 담겨있음.
- Libraries: 필요한 라이브러리 파일이 담겨있음.(OpenGL, OpenCV, Assimp 등)
- LossCalculator: 오차 계산을 위한 파이썬 프로그램이 담겨있음.
- resources: shader 파일, texture 파일, model 파일, ground truth 이미지, 본 렌더러로 렌더링한 이미지를 담고있음.
- 언급하지 않은 나머지는 연습용
