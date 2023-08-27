#include "Streamable.hpp"

using namespace hbann;

#include <format>
#include <iostream>

using namespace std;
using namespace filesystem;

#define guid_random                                                                                                    \
    {                                                                                                                  \
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10                                                                               \
    }
constexpr auto path_random = LR"(some\random.path)";

typedef struct _guid
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} guid;

wstring to_wstring(const guid &aGUID)
{
    wchar_t buffer[40];
    swprintf_s(buffer, L"{%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx}", aGUID.Data1,
               aGUID.Data2, aGUID.Data3, aGUID.Data4[0], aGUID.Data4[1], aGUID.Data4[2], aGUID.Data4[3], aGUID.Data4[4],
               aGUID.Data4[5], aGUID.Data4[6], aGUID.Data4[7]);
    return buffer;
}

class Something : public IStreamable
{
    ISTREAMABLE_DEFINE(Something, mID, mNickname, mPath, mIDK);

  public:
    Something(const guid &aID, const wstring &aNickname, const path &aPath, const size_t aAge)
        : mID(aID), mNickname(aNickname), mPath(aPath), mIDK(aAge)
    {
    }

    void Print()
    {

        wcout << vformat(L"mID = {}, mNickname = {}, mPath = {}, mIDK = {}",
                         make_wformat_args(to_wstring(mID), wstring(mNickname.begin(), mNickname.end()),
                                           mPath.wstring(), mIDK))
              << endl;
    }

  private:
    guid mID{};
    wstring mNickname{};
    path mPath{};
    size_t mIDK{};
};

int main()
{
    Something smthStart(guid_random, L"Smth", path_random, 123);
    smthStart.Print();

    Something smthEnd(smthStart.ToStream());
    smthEnd.Print();

    return 0;
}
