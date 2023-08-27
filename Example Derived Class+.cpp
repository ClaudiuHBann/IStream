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

class Rectangle : public Shape
{
    ISTREAMABLE_DEFINE_DERIVED(Rectangle, Shape, mLength, mWidth);

  public:
    Rectangle(const double aLengthWidth) : Shape(Type::SQUARE), mLength(aLengthWidth), mWidth(aLengthWidth)
    {
    }

    void Print()
    {
        Shape::Print();
        cout << format(", mLength = {}, mWidth = {}", mLength, mWidth);
    }

  private:
    double mLength{};
    double mWidth{};
};

class Square : public Rectangle
{
    ISTREAMABLE_DEFINE_DERIVED(Square, Rectangle, mDiagonal);

  public:
    Square(const double aSide) : Rectangle(aSide), mDiagonal(aSide / std::sqrt(2))
    {
    }

    void Print()
    {
        Rectangle::Print();
        cout << format(", mDiagonal = {}", mDiagonal) << endl;
    }

  private:
    double mDiagonal{};
};

int main()
{
    Shape *shapeStart3 = new Square(123.456);
    ((Square *)shapeStart3)->Print();

    Shape *shapeStart4 = new Square(shapeStart3->ToStream());
    ((Square *)shapeStart4)->Print();

    return 0;
}
