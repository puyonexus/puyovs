#pragma once
namespace ppvs
{

enum direction
{
	BELOW, ABOVE, RIGHT, LEFT
};

// (probably should have a better name... there's already std::pair for something else.)
template<class T> struct Pair
{
	T x;
	T y;
	Pair() {}
	Pair(T x, T y) :x(x), y(y) {}

	Pair<T> operator+(const Pair<T>& p1)
	{
		return Pair<T>(this->x + p1.x, this->y + p1.y);
	}
	Pair<T> operator-(const Pair<T>& p1)
	{
		return Pair<T>(this->x - p1.x, this->y - p1.y);
	}
};

typedef Pair<int> PosVectorInt;
typedef Pair<float> PosVectorFloat;

struct FieldProp
{
	int gridX, gridY; // Number of gridspaces in X and Y direction
	int gridWidth, gridHeight; // Width and height of a single gridspace
	float offsetX, offsetY; // Offset in float
	float centerX, centerY; // Center of field, eases calculation, centerY is bottom of field
	float scaleX, scaleY; // Local scale in float
	float angle;
};

}
