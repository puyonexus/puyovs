#ifndef FIELDPROP_H
#define FIELDPROP_H
namespace ppvs
{
enum direction
{
    BELOW,ABOVE,RIGHT,LEFT
};

// (probably should have a better name... there's already std::pair for something else.)
template<class T> struct pair
{
    T x;
    T y;
    pair(){}
    pair(T x,T y):x(x),y(y){}

    pair<T> operator+(const pair<T> &p1)
    {
        return pair<T>(this->x+p1.x,this->y+p1.y);
    }
    pair<T> operator-(const pair<T> &p1)
    {
        return pair<T>(this->x-p1.x,this->y-p1.y);
    }
};

typedef pair<int> posVectorInt;
typedef pair<float> posVectorFloat;


struct fieldProp
{
    int gridX,gridY; //number of gridspaces in X and Y direction
    int gridWidth,gridHeight; //width and height of a single gridspace
    float offsetX, offsetY; //offset in float
    float centerX, centerY; //center of field, eases calculation, centerY is bottom of field
    float scaleX,scaleY; //local scale in float
    float angle;
};
}
#endif // FIELDPROP_H
