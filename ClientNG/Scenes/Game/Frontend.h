#pragma once
// TODO: Fix these imports, CMake is complaining when using system imports for ALib
#include "../../../Audiolib/include/alib/audiolib.h"
#include "../../../Audiolib/include/alib/buffer.h"
#include "../../../Audiolib/include/alib/stream.h"
#include "MusicPlayer.h"
#include "../../../Puyolib/Frontend.h"
#include "../../Renderers/Renderer.h"

#include <stack>
#include <map>

namespace PuyoVS::ClientNG::Scenes::Game {

class GameSound;

class GameFrontend final : public ppvs::Frontend {
public:
	GameFrontend(Renderers::RenderTarget* target);
	~GameFrontend() override;

	ppvs::FeImage* loadImage(const char* nameu8) override;
	ppvs::FeImage* loadImage(const std::string& nameu8) override;
	ppvs::FeShader* loadShader(const char* source) override;
	ppvs::FeFont* loadFont(const char* nameu8, double fontSize) override;
	ppvs::FeSound* loadSound(const char* nameu8) override;
	ppvs::FeSound* loadSound(const std::string& nameu8) override;

	void musicEvent(ppvs::FeMusicEvent event) override;
	void musicVolume(float volume, bool fever) override;
	ppvs::FeInput inputState(int pl) override;

	void pushMatrix() override;
	void popMatrix() override;
	void identity() override;
	void translate(float x, float y, float z) override;
	void rotate(float v, float x, float y, float z) override;
	void scale(float x, float y, float z) override;

	ppvs::ViewportGeometry viewport() override;
	bool hasShaders() override;

	void setBlendMode(ppvs::BlendingMode b) override;
	void setColor(int r, int g, int b, int a) override;
	void unsetColor() override;
	void setDepthFunction(ppvs::DepthFunction func) override;
	void clearDepth() override;
	void enableAlphaTesting(float tolerance) override;
	void disableAlphaTesting() override;
	void drawRect(ppvs::FeImage* image, double subx, double suby, double subw, double subh) override;

	void clear() override;
	void swapBuffers() override;

	void setInputState(ppvs::FeInput inputState);

private:
	alib::Device* m_audioDevice;
	std::map<const char*,ppvs::FeSound*> m_audioCache;
    Renderers::RenderTarget* m_target;
	std::stack<glm::mat4> m_matrixStack;
	std::unique_ptr<Renderers::PolyBuffer> m_quadBuffer;
	std::unique_ptr<Renderers::Texture> m_blankTexture;
    Renderers::Vertex m_quadVertices[4] {};
	unsigned short m_quadIndices[6] = { 0, 1, 2, 2, 3, 0 };
	glm::vec4 m_color{};
	ppvs::FeInput m_inputState{};
};

class GameImage final : public ppvs::FeImage {
public:
	GameImage(std::unique_ptr<Renderers::Texture> texture);
	~GameImage() override;

	GameImage(const GameImage&) = delete;
	GameImage& operator=(const GameImage&) = delete;
	GameImage(GameImage&&) = delete;
	GameImage& operator=(GameImage&&) = delete;

	int width() override;
	int height() override;
	ppvs::FePixel pixel(int x, int y) override;
	bool error() override;

	void setFilter(ppvs::FilterType) override;

	void bind() { m_texture->bind(0); }

private:
	std::unique_ptr<Renderers::Texture> m_texture;
};

class GameFont final : public ppvs::FeFont {
public:
	GameFont();
	~GameFont() override;

	GameFont(const GameFont&) = delete;
	GameFont& operator=(const GameFont&) = delete;
	GameFont(GameFont&&) = delete;
	GameFont& operator=(GameFont&&) = delete;

	ppvs::FeText* render(const char* str) override;
};

class GameText final : public ppvs::FeText {
public:
	GameText();
	~GameText() override;

	GameText(const GameText&) = delete;
	GameText& operator=(const GameText&) = delete;
	GameText(GameText&&) = delete;
	GameText& operator=(GameText&&) = delete;

	void draw(float x, float y) override;
};

class GameSound final : public ppvs::FeSound {
public:
	GameSound(alib::Device* audio_device, const char* nameu8);
	~GameSound();
	void play() override;
	void stop() override;

private:
	alib::Device* m_device;
	alib::Stream m_stream;
};

}
