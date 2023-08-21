/*
    Copyright (c) 2023 Claudiu HBann

    See LICENSE for the full terms of the MIT License.
*/

#pragma once

#include <cassert>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#define ISTREAM_GET_OBJECTS_SIZE(...) hbann::IStreamUtility::FindObjectsSize(__VA_ARGS__)
#define ISTREAM_GET_OBJECTS_SIZE_DERIVED_START(...) ISTREAM_GET_OBJECTS_SIZE(__VA_ARGS__)
#define ISTREAM_GET_OBJECTS_SIZE_DERIVED(base, ...) base::GetObjectsSize() + ISTREAM_GET_OBJECTS_SIZE(__VA_ARGS__)
#define ISTREAM_GET_OBJECTS_SIZE_DERIVED_END(base, ...) ISTREAM_GET_OBJECTS_SIZE_DERIVED(base, __VA_ARGS__)

#define ISTREAM_SERIALIZE(...) IStream::InitAndWriteAll(__VA_ARGS__)
#define ISTREAM_SERIALIZE_DERIVED_START(...) ISTREAM_SERIALIZE(__VA_ARGS__)
#define ISTREAM_SERIALIZE_DERIVED(base, ...) IStream::AssignAndWriteAll(base::ToStream(), __VA_ARGS__)
#define ISTREAM_SERIALIZE_DERIVED_END(base, ...) ISTREAM_SERIALIZE_DERIVED(base, __VA_ARGS__)

#define ISTREAM_DESERIALIZE(...) IStream::ReadAllAndClear(__VA_ARGS__)
#define ISTREAM_DESERIALIZE_DERIVED_START(...) ISTREAM_DESERIALIZE_DERIVED(__VA_ARGS__)
#define ISTREAM_DESERIALIZE_DERIVED(...) IStream::ReadAll(__VA_ARGS__)
#define ISTREAM_DESERIALIZE_DERIVED_END(...) ISTREAM_DESERIALIZE(__VA_ARGS__)

namespace hbann
{
template <typename> constexpr auto always_false = false;

namespace impl
{
template <typename Type> constexpr auto is_basic_string_v = false;
template <typename... Types> constexpr auto is_basic_string_v<std::basic_string<Types...>> = true;
} // namespace impl

template <typename Type> constexpr auto is_basic_string_v = impl::is_basic_string_v<Type>;

template <typename Type>
constexpr auto is_accepted_no_range_v =
    is_basic_string_v<Type> || std::is_same_v<Type, std::filesystem::path> || std::is_standard_layout_v<Type>;
template <typename Type> constexpr auto is_accepted_v = std::ranges::range<Type> || is_accepted_no_range_v<Type>;

/**
 * @brief IStream utility class that calculates accepted objects size
 */
class IStreamUtility
{
  public:
    using type_size_sub_stream = uint32_t; // the size type of a stream inside the internal stream

    /**
     * @brief Calculates the required size in bytes to store the object in the stream
     * @note Used for any accepted type
     * @tparam Type the current object's type
     * @tparam ...Types the rest of object's types
     * @param aObject the current object
     * @param ...aObjects the rest of object
     * @return the required size in bytes to store the object in the stream
     */
    template <typename Type, typename... Types>
    static [[nodiscard]] constexpr decltype(auto) FindObjectsSize(const Type &aObject,
                                                                  const Types &...aObjects) noexcept
    {
        static_assert(is_accepted_v<Type>, "The object's type is not accepted!");

        if constexpr (std::ranges::range<Type> && !is_accepted_no_range_v<Type>)
        {
            return FindRangeSize(aObject) + FindObjectsSize(aObjects...);
        }
        else
        {
            return FindObjectSize(aObject) + FindObjectsSize(aObjects...);
        }
    }
    /**
     * @brief Calculates the required size in bytes to store the object in the stream
     * @note Used for any accepted type without ranges
     * @tparam Type the current object's type
     * @param aObject the current object
     * @return the required size in bytes to store the object in the stream
     */
    template <typename Type> static [[nodiscard]] constexpr decltype(auto) FindObjectSize(const Type &aObject) noexcept
    {
        if constexpr (is_basic_string_v<Type>)
        {
            const auto sizeInBytesOfStr = aObject.size() * sizeof(Type::value_type);
            // not a known size object so add the size in bytes of it's leading size in bytes
            return sizeof(type_size_sub_stream) + sizeInBytesOfStr;
        }
        else if constexpr (std::is_same_v<Type, std::filesystem::path>)
        {
            const auto sizeInBytesOfPath = aObject.wstring().size() * sizeof(Type::value_type);
            // not a known size object so add the size in bytes of it's leading size in bytes
            return sizeof(type_size_sub_stream) + sizeInBytesOfPath;
        }
        else if constexpr (std::is_standard_layout_v<Type>)
        {
            return sizeof(Type);
        }
        else
        {
            static_assert(std::ranges::range<Type>, "Use FindRangeSize for ranges!");
            static_assert(always_false<Type>, "The object's type is not accepted!");
        }
    }

    /**
     * @brief Get's the number of layers of a range
     * @tparam Type the range's type
     * @return the range's layers count
     */
    template <typename Type> static [[nodiscard]] constexpr size_t FindRangeLayersCount() noexcept
    {
        if constexpr (std::ranges::range<Type> && !is_accepted_no_range_v<Type>)
        {
            return 1 + FindRangeLayersCount<typename Type::value_type>();
        }
        else
        {
            return 0;
        }
    }

    /**
     * @brief Calculates the required size in bytes to store the range in the stream
     * @tparam Type the range's type
     * @param aObject the range
     * @return the required size in bytes to store the range in the stream
     */
    template <typename Type> static [[nodiscard]] constexpr size_t FindRangeSize(const Type &aObject) noexcept
    {
        if constexpr (FindRangeLayersCount<Type>())
        {
            // not a known size object so add the size in bytes of it's leading size in bytes
            return sizeof(type_size_sub_stream) +
                   std::ranges::size(aObject) * FindRangeSize(*std::ranges::cbegin(aObject));
        }
        else
        {
            return FindObjectSize(aObject);
        }
    }

    /**
     * @brief Used by FindObjectsSize(...) when there nothing to unfold
     * @return 0
     */
    static constexpr size_t FindObjectsSize() noexcept
    {
        return 0;
    }
};

/**
 * @brief Fast and easy to use single-header parser with a simple format for C++20
 *
 * TODO:
 *      - make it work with maps or pairs/tuples in general
 *      - make useful methods static and public
 *      - use ToStream for devired objects from itself
 *      - don't leave all in IStream make a IStreamWriter, IStreamReader, IStreamBase...
 *      - make ReadRange better by reserving the size of the data from the beggining like in vectors
 */
class IStream
{
    /*
        Format: [4 bytes +] any data + repeat...

            [4 bytes +] -> optional bytes that are available if *any data* is not a known size object
               any data -> any continuous stream of bytes


        ex.:
                We have a *type* that is a uint8_t and an *ID* that is string, the *mBytes* will contain:
                    - the *type* as a stream of bytes because it's a known size object
                    - the size in bytes of *ID* as bytes because it's not a known size object followed by the *ID* as a
                    stream of bytes


                uint8_t type = 24;
                string ID = "ea025860-02bd-47cb-b053-b82ca250e4ba";

               1 byte   +  4 bytes  +     36 bytes
                0x18    +   0x24    +   *ID* as bytes
    */
    std::vector<uint8_t> mStream{};

  public:
    using type_size_sub_stream = IStreamUtility::type_size_sub_stream;
    using type_stream = decltype(mStream);
    using type_stream_value = type_stream::value_type;

    /**
     * @brief Default constructor used with ToStream
     */
    constexpr IStream() noexcept = default;

    /**
     * @brief Converts the stream to an object
     * @note Flow with simple classes: ISTREAM_SERIALIZE(...) in class
     * @note Flow with base/derived classes:
     * base class will use ISTREAM_DESERIALIZE_DERIVED_START(...)
     * middle derived classes will use ISTREAM_DESERIALIZE_DERIVED(...)
     * last derived class will use ISTREAM_DESERIALIZE_DERIVED_END(...)
     * @param aStream the object as a rvalue stream
     */
    constexpr explicit IStream(type_stream &&aStream) noexcept
    {
        Assign(move(aStream));
    }

    /**
     * @brief Uhmm, just a destructor..
     */
    constexpr virtual ~IStream() noexcept = default;

    /**
     * @brief Converts the object to a stream
     * @note Flow with simple classes: ISTREAM_DESERIALIZE(...) in class
     * @note Flow with base/derived classes:
     * base class will use ISTREAM_SERIALIZE_DERIVED_START(...)
     * middle derived classes will use ISTREAM_SERIALIZE_DERIVED(...)
     * last derived class will use ISTREAM_SERIALIZE_DERIVED_END(...)
     * @return the object as a rvalue stream
     */
    virtual [[nodiscard]] type_stream &&ToStream() = 0;

    // C++20 magic
    auto operator<=>(const IStream &) const = default;

  protected:
    /**
     * @brief Gets the required size to store the object
     */
    virtual constexpr size_t GetObjectsSize() const noexcept = 0;

    /**
     * @brief Initializes the stream and writes all the objects to it
     * @return the rvalue stream
     */
    template <typename... Types> [[nodiscard]] constexpr decltype(auto) InitAndWriteAll(const Types &...aObjects)
    {
        Init();
        WriteAll(aObjects...);
        return Release();
    }

    /**
     * @brief Assigns the stream and writes all the objects to it
     * @tparam ...Types the rest of object's types
     * @param aStream the rvalue stream
     * @param ...aObjects the rest of object
     * @return the rvalue stream
     */
    template <typename... Types>
    [[nodiscard]] constexpr decltype(auto) AssignAndWriteAll(type_stream &&aStream, const Types &...aObjects)
    {
        Assign(move(aStream), false);
        WriteAll(aObjects...);
        return Release();
    }

    /**
     * @brief Reads the objects from the stream and clears it
     * @tparam ...Types the objects's type
     * @param ...aObjects the object be read
     */
    template <typename... Types> constexpr void ReadAllAndClear(Types &...aObjects)
    {
        ReadAll(aObjects...);
        Clear();
    }

    /**
     * @brief Reads all the object from the stream
     * @tparam Type the current object's type
     * @tparam ...Types the rest of object's types
     * @param aObject the current object
     * @param ...aObjects the rest of object
     */
    template <typename Type, typename... Types> constexpr void ReadAll(Type &aObject, Types &...aObjects) noexcept
    {
        aObject = Read<Type>();

        if constexpr (sizeof...(aObjects))
        {
            ReadAll(aObjects...);
        }
    }

  private:
    size_t mIndex{};

    /**
     * @brief Allocates memory for the fixed size stream
     * @note Finds the size automatically for derived classes
     */
    constexpr void Init()
    {
        mStream = type_stream();
        mStream.reserve(GetObjectsSize());
        mIndex = {};
    }

    /**
     * @brief Assigns the stream
     * @param aStream the rvalue stream
     * @param aReset should reset the stream's index
     */
    constexpr void Assign(type_stream &&aStream, const bool aReset = true) noexcept
    {
        if (aReset)
        {
            mIndex = {};
        }

        mStream = move(aStream);
    }

    /**
     * @brief Writes the object in the stream
     * @tparam Type the object's type
     * @param aObject the object
     * @param aSize the object's size in bytes
     * @note aSize is required for unhandled types
     */
    template <typename Type = void *> constexpr void Write(const Type &aObject, const type_size_sub_stream aSize = 0)
    {
        static_assert(is_accepted_v<Type>, "The object's type is not accepted!");

        if constexpr (is_basic_string_v<Type>)
        {
            auto size = type_size_sub_stream(aObject.size() * sizeof(Type::value_type));
            WriteObject(aObject.data(), size);
        }
        else if constexpr (std::is_same_v<Type, std::filesystem::path>)
        {
            Write(aObject.wstring());
        }
        else if constexpr (std::is_standard_layout_v<Type>)
        {
            WriteObjectOfKnownSize(&aObject, sizeof(Type));
        }
        // last check because types like string and path are ranges
        else if constexpr (std::ranges::range<Type>)
        {
            WriteRange(aObject);
        }
        else
        {
            WriteObject(&aObject, aSize);
        }
    }

    /**
     * @brief Writes all the object in the stream
     * @tparam Type the current object's type
     * @tparam ...Types the rest of object's types
     * @param aObject the current object
     * @param ...aObjects the rest of object
     */
    template <typename Type, typename... Types> constexpr void WriteAll(const Type &aObject, const Types &...aObjects)
    {
        Write(aObject);

        if constexpr (sizeof...(aObjects))
        {
            WriteAll(aObjects...);
        }
    }

    /**
     * @brief Reads the object from the stream
     * @note for unhandled object types will return a span
     * @tparam Type the object's type
     * @return the object
     */
    template <typename Type = std::span<type_stream_value>> [[nodiscard]] constexpr decltype(auto) Read() noexcept
    {
        static_assert(is_accepted_v<Type>, "The object's type is not accepted!");

        if constexpr (is_basic_string_v<Type>)
        {
            const auto [ptr, size] = ReadStream<Type>();
            return Type(ptr, size);
        }
        else if constexpr (std::is_same_v<Type, std::filesystem::path>)
        {
            const auto [ptr, size] = ReadStream<Type>();
            return basic_string<std::filesystem::path::value_type>(ptr, size);
        }
        // std::is_standard_layout_v is true for span but we want the last branch for spans so:
        else if constexpr (std::is_standard_layout_v<Type> && !std::is_same_v<Type, std::span<type_stream_value>>)
        {
            return ReadObjectOfKnownSize<Type>();
        }
        // last check because types like string and path are ranges
        else if constexpr (std::ranges::range<Type>)
        {
            return ReadRange<Type>();
        }
        else
        {
            return ReadObject();
        }
    }

    /**
     * @brief Releases the stream
     * @return the rvalue stream
     */
    [[nodiscard]] constexpr decltype(auto) Release() noexcept
    {
        return move(mStream);
    }

    /**
     * @brief Clears the stream and it's index
     */
    constexpr void Clear() noexcept
    {
        mStream.clear();
        mIndex = {};
    }

    /**
     * @brief Writes a number of bytes from stream
     * @param aStream the stream
     * @param aSize the number of bytes to write
     */
    void WriteObject(const void *aStream, const type_size_sub_stream aSize)
    {
        WriteSize(aSize);

        // write the stream
        const auto streamPtr = reinterpret_cast<const type_stream_value *>(aStream);
        mStream.insert(mStream.end(), streamPtr, streamPtr + aSize);
        mIndex += aSize;
    }

    /**
     * @brief Writes a number of bytes from stream
     * @note the object's type must be a known size type
     * @param aStream the stream
     * @param aSize the number of bytes to write
     */
    void WriteObjectOfKnownSize(const void *aStream, const type_size_sub_stream aSize)
    {
        assert(aSize);

        const auto streamPtr = reinterpret_cast<const type_stream_value *>(aStream);
        mStream.insert(mStream.end(), streamPtr, streamPtr + aSize);
        mIndex += aSize;
    }

    /**
     * @brief Writes any nested range to the stream
     * @tparam Type the nested range object type
     */
    template <std::ranges::range Range> constexpr void WriteRange(const Range &aRange)
    {
        WriteSize(type_size_sub_stream(std::ranges::size(aRange)));
        if constexpr (!is_accepted_no_range_v<Range> && IStreamUtility::FindRangeLayersCount<Range>() > 1)
        {
            std::ranges::for_each(aRange, [this](const auto &aObject) { WriteRange(aObject); });
        }
        else
        {
            std::ranges::for_each(aRange, [this](const auto &aObject) { Write(aObject); });
        }
    }

    /**
     * @brief Reads any nested range from the stream
     * @tparam Type the nested range object type
     * @return the nested range object
     */
    template <typename Type> [[nodiscard]] constexpr decltype(auto) ReadRange()
    {
        Type object{};
        const auto size = ReadSize();

        if constexpr (IStreamUtility::FindRangeLayersCount<Type>() > 1)
        {
            for (size_t i = 0; i < size; i++)
            {
                object.insert(std::ranges::cend(object), ReadRange<typename Type::value_type>());
            }
        }
        else
        {
            for (size_t i = 0; i < size; i++)
            {
                object.insert(std::ranges::cend(object), Read<typename Type::value_type>());
            }
        }

        return object;
    }

    /**
     * @brief Writes the size as bytes to the stream
     * @param aSize the size
     */
    void WriteSize(const type_size_sub_stream aSize)
    {
        assert(aSize);

        // write the stream's size as bytes
        const auto sizePtr = reinterpret_cast<const type_stream_value *>(&aSize);
        mStream.insert(mStream.end(), sizePtr, sizePtr + sizeof(type_size_sub_stream));
        mIndex += sizeof(type_size_sub_stream);
    }

    /**
     * @brief Reads the size of the current sub stream inside the stream
     * @return the current sub stream size
     */
    [[nodiscard]] decltype(auto) ReadSize() noexcept
    {
        assert(mIndex + sizeof(type_size_sub_stream) <= mStream.size());

        const auto sizePtr = reinterpret_cast<type_size_sub_stream *>(mStream.data() + mIndex);
        mIndex += sizeof(type_size_sub_stream);
        return *sizePtr;
    }

    /**
     * @brief Reads an object from the stream
     * @tparam Type the stream's type
     * @return a pair containing the stream start and it's size
     */
    template <typename Type> constexpr [[nodiscard]] decltype(auto) ReadStream() noexcept
    {
        const auto spen(ReadObject());
        const auto streamType = reinterpret_cast<Type::value_type *>(spen.data());
        const auto streamSize = spen.size_bytes() / sizeof(Type::value_type);

        return std::pair{streamType, streamSize};
    }

    /**
     * @brief Reads an object from the stream
     * @return a span containing the object as a stream
     */
    [[nodiscard]] decltype(auto) ReadObject() noexcept
    {
        const auto size = ReadSize();
        assert(mIndex + size <= mStream.size());

        std::span<type_stream_value> spen(mStream.data() + mIndex, size);
        mIndex += size;
        return spen;
    }

    /**
     * @brief Reads an object from stream
     * @note the object's type must be a known size type
     * @return the object
     */
    template <typename Type> [[nodiscard]] constexpr decltype(auto) ReadObjectOfKnownSize() noexcept
    {
        assert(mIndex + sizeof(Type) <= mStream.size());

        const auto objectPtr = reinterpret_cast<Type *>(mStream.data() + mIndex);
        mIndex += sizeof(Type);
        return *objectPtr;
    }
};
} // namespace hbann
