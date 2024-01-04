find_package(OpenGL REQUIRED COMPONENTS OpenGL)

add_executable(vs2 WIN32
	"main.cpp"
	"Scenes/Intro/Logo.cpp"
	"Renderers/OpenGL/OpenGL.cpp"
	"Renderers/Renderer.h"
	"Scenes/Scene.h"
	"Scenes/Intro/IntroLogo.cpp"
	"Scenes/Intro/IntroLogo.h"
	"Scenes/Game/InGame.cpp"
	"Game.cpp"
	"Game.h"
	"Renderers/Renderer.cpp"
	"PTSans.cpp"
	"PTSans.h"
	"Font.cpp"
	"Font.h"
	"Scenes/Intro/IntroNotes.cpp"
	"Scenes/Game/Frontend.cpp"
	"Scenes/Game/Frontend.h"
	"Paths.h"
	"Paths.cpp"
	"Renderers/Software/Software.cpp"
	"Renderers/Software/Software.h"
)

target_compile_features(vs2 PUBLIC cxx_std_17)

target_link_libraries(vs2
	Audiolib
	JsonCpp
	Puyolib
	PVS_ENet
	SDL2-static
	SDL2main
	SDL2_ttf::SDL2_ttf-static
	ZLIB::ZLIB
	spng
	glm::glm
	${OPENGL_LIBRARIES}
)
