#include "IStream.hpp"

using namespace hbann;

#include <format>
#include <iostream>

using namespace std;

class Shape : public IStream
{
  public:
    enum class Type : uint8_t
    {
        UNKNOWN,
        RECTANGLE,
        SQUARE,
        CIRCLE
    };

    Shape(type_stream &&aStream) : IStream(move(aStream))
    {
        ISTREAM_DESERIALIZE_DERIVED_START(mType);
    }

    Shape(const Type &aType) : mType(aType)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAM_SERIALIZE_DERIVED_START(mType);
    }

    void Print()
    {
        cout << format("mType = {}", (uint8_t)mType);
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAM_GET_OBJECTS_SIZE_DERIVED_START(mType);
    }

  private:
    Type mType = Type::UNKNOWN;
};

class Circle : public Shape
{
  public:
    Circle(type_stream &&aStream) : Shape(move(aStream))
    {
        ISTREAM_DESERIALIZE_DERIVED_END(mRadius);
    }

    Circle(const double aRadius) : Shape(Type::CIRCLE), mRadius(aRadius)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAM_SERIALIZE_DERIVED_END(Shape, mRadius);
    }

    void Print()
    {
        Shape::Print();
        cout << format(", mRadius = {}", mRadius) << endl;
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAM_GET_OBJECTS_SIZE_DERIVED_END(Shape, mRadius);
    }

  private:
    double mRadius{};
};

int main()
{
    // devired class IStream manipulation 2
    Shape *shapeStart1 = new Circle(3.14156);
    ((Circle *)shapeStart1)->Print();

    Shape *shapeStart2 = new Circle(shapeStart1->ToStream());
    ((Circle *)shapeStart2)->Print();

    return 0;
}
