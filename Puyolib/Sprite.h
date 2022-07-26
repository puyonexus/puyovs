#pragma once

#include "FieldProp.h"
#include "Frontend.h"

namespace ppvs
{

class FeRenderTarget;
class FeImage;

class Sprite
{
public:
	Sprite();
	~Sprite();

	bool m_visible;
	bool m_flipX;
	bool m_flipY;
	float m_posX, m_posY;
	float m_scaleX, m_scaleY;
	int m_subRectX, m_subRectY, m_subRectW, m_subRectH;
	int m_centerX, m_centerY;
	int m_colorA, m_colorR, m_colorG, m_colorB;
	float m_rotation;
	BlendingMode m_blendMode;

	// Setters
	void setImage(FeImage* img);
	void setX(float);
	void setY(float);
	void addX(float);
	void addY(float);
	void setSubRect(int x, int y, int width, int height);
	void setCenter();
	void setCenter(int x, int y);
	void setCenter(PosVectorFloat pv);
	void setCenterBottom();
	void setScale(float scale);
	void setScaleX(float scale);
	void setScaleY(float scale);
	void setScale(float x, float y);
	void setSize(float x, float y);
	void setSize(PosVectorFloat pv);
	void setPosition(float, float);
	void setPosition(PosVectorFloat pv);
	void setRotation(float);
	void setTransparency(float);
	void setColor(float r, float g, float b, float a);
	void setColor(float r, float g, float b);
	void setVisible(bool);
	void setBlendMode(BlendingMode);
	void setFlipX(bool f);
	void setFlipY(bool f);

	void draw(FeRenderTarget* target, FeShader* shader = nullptr);
	void redraw(FeRenderTarget* target); // Draw again with blend mode add

	[[nodiscard]] float getSizeX() const;
	[[nodiscard]] float getSizeY() const;
	[[nodiscard]] float getScaleX() const;
	[[nodiscard]] float getScaleY() const;
	[[nodiscard]] PosVectorFloat getPosition() const;
	[[nodiscard]] float getX() const;
	[[nodiscard]] float getY() const;
	[[nodiscard]] float getAngle() const;
	[[nodiscard]] float getTransparency() const;
	[[nodiscard]] FeImage* getImage() const;

private:
	void setColor();
	FeImage* m_image;
	int m_colorRInternal, m_colorGInternal, m_colorBInternal, m_colorAInternal;
};

}
