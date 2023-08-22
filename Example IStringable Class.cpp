#include "Streamable.hpp"

using namespace hbann;

#include <iostream>

using namespace std;

class Range : public IStreamable
{
  public:
    Range() = default;

    explicit Range(type_stream &&aStream) : IStreamable(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED_START(mInts);
    }

    Range(const vector<int> &aInts) : mInts(aInts)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED_START(mInts);
    }

    void Print()
    {
        cout << "mInts = { ";
        for (const auto i : mInts)
        {
            cout << i << " ";
        }
        cout << "}" << endl;
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED_START(mInts);
    }

  private:
    vector<int> mInts{};
};

class NestedRange : public Range
{
  public:
    NestedRange() = default;

    explicit NestedRange(type_stream &&aStream) : Range(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE_DERIVED_END();
    }

    NestedRange(const vector<int> &aInts) : Range(aInts)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE_DERIVED_END(Range);
    }

    void Print()
    {
        Range::Print();
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE_DERIVED_END(Range);
    }
};

class Something : public IStreamable
{
  public:
    explicit Something(type_stream &&aStream) : IStreamable(move(aStream))
    {
        ISTREAMABLE_DESERIALIZE(mNestedRange);
    }

    Something(const vector<int> &aNestedRange) : mNestedRange(aNestedRange)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE(mNestedRange);
    }

    void Print()
    {
        mNestedRange.Print();
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE(mNestedRange);
    }

  private:
    NestedRange mNestedRange;
};

int main()
{
    Something smthStart(vector{1, 2, 3});
    smthStart.Print();

    Something smthEnd(smthStart.ToStream());
    smthEnd.Print();

    return 0;
}
