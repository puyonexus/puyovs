#pragma once
namespace ppvs
{

enum direction
{
	BELOW, ABOVE, RIGHT, LEFT
};

// (probably should have a better name... there's already std::pair for something else.)
template<class T> struct pair
{
	T x;
	T y;
	pair() {}
	pair(T x, T y) :x(x), y(y) {}

	pair<T> operator+(const pair<T>& p1)
	{
		return pair<T>(this->x + p1.x, this->y + p1.y);
	}
	pair<T> operator-(const pair<T>& p1)
	{
		return pair<T>(this->x - p1.x, this->y - p1.y);
	}
};

typedef pair<int> posVectorInt;
typedef pair<float> posVectorFloat;


struct fieldProp
{
	int gridX, gridY; // Number of gridspaces in X and Y direction
	int gridWidth, gridHeight; // Width and height of a single gridspace
	float offsetX, offsetY; // Offset in float
	float centerX, centerY; // Center of field, eases calculation, centerY is bottom of field
	float scaleX, scaleY; // Local scale in float
	float angle;
};

}
