# Streamable

This library is all about speedy data parsing, churning out super small byte streams.

## Table of Contents

- [Installation](#installation)
- [Features](#features)
- [Usage](#usage)
- [Examples](#examples)
- [Documentation](#documentation)
- [TODO](#todo)

## Installation

To use this library, simply get and include the header file `Streamable.hpp` into your project.

## Features

- **fast** - the parsing represents just a simple iteration, knows where every object is and how big it is, for example it reserves the memory for ranges that allow it before adding elements etc...
- **easy-to-use** - inherit a class and implement it's methods with just a macro
- **single-header** - just copy paste the file in your project
- **simple format** - contains just the data itself and for the types that have a dynamic size a metadata representing just a uint32_t
- **has no dependencies** - uses just the standard library
- **accepts multiple data types** - beside **itself** ofc, **primitive types** (ex.: bool, unsigned int, double etc...), **strings** (ex.: std::string. std::wstring etc...), **any type with standard layout** (ex.: POD structs and classes, enums, etc...), **nested ranges** (ex.: vector, list, vector&lt;list&gt; etc...), and bonus types like *std::filesystem::path* etc...

## Usage

1. Inherit from the `IStreamable` class or any class that implements it.
2. Implement the `Constructor(stream)` used for deserializing, simply by using the macro ISTREAMABLE_DESERIALIZE_X(object1, object2, ...)
3. Implement the `ToStream()` method used for serializing, simply by using the macro ISTREAMABLE_SERIALIZE_X(object1, object2, ...)
4. Implement the `GetObjectsSize` used for calculating the exact size required to store the objects, simply by using the macro ISTREAMABLE_GET_OBJECTS_SIZE_X(object1, object2, ...)

## Examples

- [Simple Class](https://github.com/ClaudiuHBann/Streamable/blob/main/Example%20Simple%20Class.cpp) - how to use **Streamable** for a simple class
- [Derived Class](https://github.com/ClaudiuHBann/Streamable/blob/main/Example%20Derived%20Class.cpp) - how to use **Streamable** for a base class and a derived class
- [Derived Classes](https://github.com/ClaudiuHBann/Streamable/blob/main/Example%20Derived%20Class%2B.cpp) - how to use **Streamable** for a base class, multiple intermediate classes and the final class

## Documentation

There are 3 types of macros:
- **ISTREAMABLE_GET_OBJECTS_SIZE_X** - finds the exact size required to store the objects
- **ISTREAMABLE_SERIALIZE_X** - serializes the objects
- **ISTREAMABLE_DESERIALIZE_X** - deserializes the objects

Those 3 macros have 4 types each that will be used depending on the situation:
- **ISTREAMABLE_X**(...) - used by simple classes
- **ISTREAMABLE_X_DERIVED_START**(...) - used by the base classes
- **ISTREAMABLE_X_DERIVED**(...) - used by the intermediate classes
- **ISTREAMABLE_X_DERIVED_END**(...) - used by the final classes

## TODO

Features:
- make it work with tuples

Enchantments:
- remove the single copy from the project from ReadStreamable
- split the class IStreamable into IStreamWriter, IStreamReader, IStreamBase...

Bugs:
- give me some... :)
