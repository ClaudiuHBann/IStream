#include "Streamable.hpp"

using namespace hbann;

#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

template <typename Object, typename Iterable>
void Print(
    const Iterable &iterable, const string &separatorDimensions = "\n",
    const function<void(const Object &)> &funcPrintElem = [](const Object &obj) {
        static_assert(
            is_arithmetic_v<Object> || is_same_v<remove_const_t<remove_pointer_t<Object>>, char>,
            R"(The object from the innermost range is not a built-in/c-string type, please provide a valid print element function.)");
        cout << obj << ' ';
    })
{
    if constexpr (ranges::range<Iterable>)
    {
        ranges::for_each(iterable, [&](const auto &it) { Print(it, separatorDimensions, funcPrintElem); });
        cout << separatorDimensions;
    }
    else
    {
        funcPrintElem(iterable);
    }
}

#include <set>

class NestedRange : public IStreamable
{
  public:
    explicit NestedRange(type_stream &&aStream) : IStreamable(std::move(aStream))
    {
        ISTREAMABLE_DESERIALIZE(mInts);
    }

    NestedRange(const vector<list<set<double>>> &aInts) : mInts(aInts)
    {
    }

    type_stream &&ToStream() override
    {
        return ISTREAMABLE_SERIALIZE(mInts);
    }

    void Print() const noexcept
    {
        ::Print<double>(mInts);
    }

  protected:
    constexpr size_t GetObjectsSize() const noexcept override
    {
        return ISTREAMABLE_GET_OBJECTS_SIZE(mInts);
    }

  private:
    vector<list<set<double>>> mInts{};
};

int main()
{
    NestedRange nestedRangeStart({{{1.23, 2.3, 3.0}, {1.23, 2.3, 3.0}, {1.23, 2.3, 3.0}},
                                  {{1.23, 2.3, 3.0}, {1.23, 2.3, 3.0}, {1.23, 2.3, 3.0}}});
    nestedRangeStart.Print();

    NestedRange nestedRangeEnd(nestedRangeStart.ToStream());
    nestedRangeEnd.Print();

    return 0;
}
