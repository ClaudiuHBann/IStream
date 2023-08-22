#include "Streamable.hpp"

using namespace hbann;

#include <format>
#include <iostream>

using namespace std;

class Shape : public IStreamable
{
  public:
    enum class Type : uint8_t
    {
        UNKNOWN,
        RECTANGLE,
        SQUARE,
        CIRCLE
    };

    Shape(type_stream &&aStream) : IStreamable(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED_START(mType);
    }

    Shape(const Type &aType) : mType(aType)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED_START(mType);
    }

    void Print()
    {
        cout << format("mType = {}", (uint8_t)mType);
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED_START(mType);
    }

  private:
    Type mType = Type::UNKNOWN;
};

class Rectangle : public Shape
{
  public:
    Rectangle(type_stream &&aStream) : Shape(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED(mLength, mWidth);
    }

    Rectangle(const double aLengthWidth) : Shape(Type::SQUARE), mLength(aLengthWidth), mWidth(aLengthWidth)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED(Shape, mLength, mWidth);
    }

    void Print()
    {
        Shape::Print();
        cout << format(", mLength = {}, mWidth = {}", mLength, mWidth);
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED(Shape, mLength, mWidth);
    }

  private:
    double mLength{};
    double mWidth{};
};

class Square : public Rectangle
{
  public:
    Square(type_stream &&aStream) : Rectangle(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED_END(mDiagonal);
    }

    Square(const double aSide) : Rectangle(aSide), mDiagonal(aSide / std::sqrt(2))
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED_END(Rectangle, mDiagonal);
    }

    void Print()
    {
        Rectangle::Print();
        cout << format(", mDiagonal = {}", mDiagonal) << endl;
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED_END(Rectangle, mDiagonal);
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
