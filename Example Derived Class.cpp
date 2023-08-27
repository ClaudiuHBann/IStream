#include "Streamable.hpp"

using namespace hbann;

#include <format>
#include <iostream>

using namespace std;

class Shape : public IStreamable
{
    ISTREAMABLE_DEFINE_DERIVED_START(Shape, mType);

  public:
    enum class Type : uint8_t
    {
        UNKNOWN,
        RECTANGLE,
        SQUARE,
        CIRCLE
    };

    Shape(const Type &aType) : mType(aType)
    {
    }

    void Print()
    {
        cout << format("mType = {}", (uint8_t)mType);
    }

  private:
    Type mType = Type::UNKNOWN;
};

class Circle : public Shape
{
    ISTREAMABLE_DEFINE_DERIVED_END(Circle, Shape, mRadius);

  public:
    Circle(const double aRadius) : Shape(Type::CIRCLE), mRadius(aRadius)
    {
    }

    void Print()
    {
        Shape::Print();
        cout << format(", mRadius = {}", mRadius) << endl;
    }

  private:
    double mRadius{};
};

int main()
{
    Shape *shapeStart1 = new Circle(3.14156);
    ((Circle *)shapeStart1)->Print();

    Shape *shapeStart2 = new Circle(shapeStart1->ToStream());
    ((Circle *)shapeStart2)->Print();

    return 0;
}
