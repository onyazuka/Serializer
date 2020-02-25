# Serializer
Simple C++ serializer

<h2>Description</h2>
Simple c++ serialization library.
It allows you to serialize such types:
<ul>
<li>Simple arithmetic types</li>
<li>Arrays</li>
<li>All std containers</li>
<li>Your custom types</li>
</ul>
It makes a std::string, filled with binary data, on output. Why use std::string? Later, you can write it to a file or make some manipulations and write it somewhere after that.

<h2>Requirements</h2>
It is just one header, without any dependencies. So, any c++ compiler that supports c++17 should be ok, but I have only tested it with g++.

<h2>Examples</h2>
First of all you should include "serializer.hpp" header and, if you want, do "using namespace Serialization;".

Then, let's try to serialize some simple type:
```Cpp
int i = 5; 							// your value
std::string buf;		// your buffer
BytesCount b = Serializer::SerializeUnit<int>::serializeUnit(buf, &i);
```
Here, 'b' is the number of bytes that was read;

And after that, you can try to get your number back:
```Cpp
int i1;
b = Deserializer::DeserializeUnit<int>::deserializeUnit(buf, 0, &i);
```
Here, the second argument of deserializeUnit is the offset, from which value in the 'buf' will be looked for.

It is very convenient to use the 'serializeAll' function. It uses variadic templates, and with it you could serialize a bunch of things just in one line:
```Cpp
buf.clear();
double d = 1.1;
std::unordered_map<int, std::string> um{{1, "neko"}, {2, "wanko"}, {3, "manko"}};
std::vector<std::vector<std::string>> vvs{{"some", "thing"}, {"some", "other", "thing"}};
b = Serializer::serializeAll(buf, &um, &d, &vvs);

// getting back
decltype(um) um1;
decltype(vvs) vvs1;
decltype(d) d1;

b = Deserializer::deserializeAll(buf, 0, &um1, &d1, &vvs1);
```
To serialize array types(including std::array), you shoud use ArrayWrapper<T> from Serialization namespace:
```Cpp
buf.clear();
int* pi = new int[10];
... // initializng 'i'
ArrayWrapper aw{pi, 10};
b = Serializer::serializeAll(buf, &aw);
```
As you can see, pointers are used as arguments, so if you try to pass a raw pointer as a parameter, just the first value of this type starting from the passed address will be processed.

And, to get it back, we should use ArrayWrapper again.
WARNING: before deserialization, memory for array should be already allocated:
```Cpp
ArrayWrapper<int> aw1;
aw11.start = new int[5];
b = Deserializer::deserializeAll(buf, 0, &aw1);
```

And, of course, you can serialize your own objects. Here you have two options:
- If serialization/deserialization of your object can be done only in one way, you should inherit Serializable/Deserializable class(or both):
```Cpp
struct User : public Serialization::Serializable, public Serialization::Deserializable
{
    std::string name;
    int age;
    std::vector<std::string> hobbies;

    S::BytesCount serialize(std::string &buf)
    {
        return S::Serializer::serializeAll(buf, &name, &age, &hobbies);
    }

    S::BytesCount deserialize(const std::string& buf, S::BytesCount offset)
    {
        return S::Deserializer::deserializeAll(buf, offset, &name, &age, &hobbies);
    }
};

...

User user;
user.age = 22;
user.name = "Vasya";
user.hobbies = {"swimming", "anime", "fishing"};

std::string buf;
BytesCount b = Serializer::serializeAll(buf, &user);

```

- If serialization/deserialization of your object can be done in multiple ways, you should inherit MultipleSerializable/MultipleDeserializable(or both):
```Cpp
struct Cat : public MultipleSerializable, public MultipleDeserializable
{
    enum {NameLegsSerializer, AllSerializer, NameLegsDeserializer, AllDeserializer};
    Cat()
        : MultipleSerializable{}, MultipleDeserializable{}
    {
        registerSerializer(SerializerId(NameLegsSerializer), [this](std::string& buf) { return Serializer::serializeAll(buf, &name, &legs); } );
        registerSerializer(SerializerId(AllSerializer), [this](std::string& buf) { return Serializer::serializeAll(buf, &name, &legs, &age, &places_to_sleep); } );
        registerDeserializer(SerializerId(NameLegsDeserializer), [this](const std::string& buf, BytesCount offset) { return Deserializer::deserializeAll(buf, offset, &name, &legs); } );
        registerDeserializer(SerializerId(AllDeserializer), [this](const std::string& buf, BytesCount offset) { return Deserializer::deserializeAll(buf, offset, &name, &legs, &age, &places_to_sleep); } );
    }
    std::string name;
    int legs;
    short age;
    std::vector<std::string> places_to_sleep;
};

...

Cat cat;
cat.age = 5;
cat.legs = 4;
cat.name = "Sugrob";
cat.places_to_sleep = std::move(std::vector<std::string>{"chair", "laptop", "my head"});
cat.setSerializerId((SerializerId)(Cat::AllSerializer));

Serializer::serializeAll(buf, &cat);
```

