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

class Circle : public Shape
{
  public:
    Circle(type_stream &&aStream) : Shape(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED_END(mRadius);
    }

    Circle(const double aRadius) : Shape(Type::CIRCLE), mRadius(aRadius)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED_END(Shape, mRadius);
    }

    void Print()
    {
        Shape::Print();
        cout << format(", mRadius = {}", mRadius) << endl;
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED_END(Shape, mRadius);
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
