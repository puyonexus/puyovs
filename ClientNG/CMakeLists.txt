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
	"Scenes/Game/MusicPlayer.cpp"
	"Scenes/Game/MusicPlayer.h"
)

target_compile_features(vs2 PUBLIC cxx_std_17)

target_link_libraries(vs2
	Audiolib::Audiolib
	JsonCpp::JsonCpp
	ZLIB::ZLIB
	Puyolib::Puyolib
	PVS_ENet
	$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
	$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
	$<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
	spng::spng
	glm::glm
	${OPENGL_LIBRARIES}
)
