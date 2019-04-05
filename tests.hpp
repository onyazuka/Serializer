#pragma once
#include <gtest/gtest.h>
#include <unordered_map>
#include "serializer.hpp"

namespace S = Serialization;


template<typename Cont>
bool areContainersEqual(const Cont& c1, const Cont& c2)
{
    if(c1.size() != c2.size()) return false;
    auto iter1 = c1.begin();
    auto iter2 = c2.begin();
    while(iter1 != c1.end() && iter2 != c2.end())
    {
        if(*(iter1++) != *(iter2++)) return false;
    }
    return true;
}

struct User : public Serialization::Serializable, public Serialization::Deserializable
{
    std::string name;
    int age;
    std::vector<std::string> hobbies;

    S::BytesCount serialize(std::string &buf)
    {
        return S::Serializer::serializeAll(buf, &name, &age, &hobbies);
    }

    S::BytesCount deserialize(std::string& buf, S::BytesCount offset)
    {
        return S::Deserializer::deserializeAll(buf, offset, &name, &age, &hobbies);
    }

    /*std::unordered_map<S::SerializerId, S::SerializerFunction> sMap;
    std::unordered_map<S::SerializerId, S::DeserializerFunction> dsMap;*/

};

struct Cat : public S::MultipleSerializable, public S::MultipleDeserializable
{
    enum {NameLegsSerializer, AllSerializer, NameLegsDeserializer, AllDeserializer};
    Cat()
        : S::MultipleSerializable{}, S::MultipleDeserializable{}
    {
        registerSerializer(S::SerializerId(NameLegsSerializer), [this](std::string& buf) { return S::Serializer::serializeAll(buf, &name, &legs); } );
        registerSerializer(S::SerializerId(AllSerializer), [this](std::string& buf) { return S::Serializer::serializeAll(buf, &name, &legs, &age, &places_to_sleep); } );
        registerDeserializer(S::SerializerId(NameLegsDeserializer), [this](std::string& buf, S::BytesCount offset) { return S::Deserializer::deserializeAll(buf, offset, &name, &legs); } );
        registerDeserializer(S::SerializerId(AllDeserializer), [this](std::string& buf, S::BytesCount offset) { return S::Deserializer::deserializeAll(buf, offset, &name, &legs, &age, &places_to_sleep); } );
    }
    std::string name;
    int legs;
    short age;
    std::vector<std::string> places_to_sleep;

    S::BytesCount serialize(std::string &buf)
    {
        return getCurrentSerializer()(buf);
    }

    S::BytesCount deserialize(std::string &buf, S::BytesCount offset)
    {
        return getCurrentDeserializer()(buf, offset);
    }
};

TEST(BasicTypesTest, SerializerTests)
{
    using namespace Serialization;

    int i = 1;
    double d = 2.5;
    std::string str = "Neko";
    std::vector<int> v{1,2,3};

    std::string buf;


    Serializer::SerializeUnit<int>::serializeUnit(buf, &i);
    EXPECT_EQ(buf.size(), sizeof(i));
    Serializer::SerializeUnit<double>::serializeUnit(buf, &d);
    EXPECT_EQ(buf.size(), sizeof(i) + sizeof(d));

    //s.serializeUnit(str, buf);    // error
    //s.serializeUnit(v, buf);      // error

    int i1;
    double d1;
    BytesCount offset = 0;
    offset += Deserializer::DeserializeUnit<int>::deserializeUnit(buf, offset, &i1);
    EXPECT_EQ(offset, sizeof(i));
    offset += Deserializer::DeserializeUnit<double>::deserializeUnit(buf, offset, &d1);

    EXPECT_EQ(i, i1);
    EXPECT_EQ(d, d1);

    buf.clear();
    int ii = 22, ii1;
    double dd = 22.22, dd1;
    short ss = 3, ss1;
    uint64_t uu = 88888888888888, uu1;

    Serializer::SerializeUnit<int>::serializeUnit(buf, &ii);
    Serializer::SerializeUnit<double>::serializeUnit(buf, &dd);
    Serializer::SerializeUnit<short>::serializeUnit(buf, &ss);
    Serializer::SerializeUnit<uint64_t>::serializeUnit(buf, &uu);

    offset = 0;
    offset += Deserializer::DeserializeUnit<int>::deserializeUnit(buf, offset, &ii1);
    offset += Deserializer::DeserializeUnit<double>::deserializeUnit(buf, offset, &dd1);
    offset += Deserializer::DeserializeUnit<short>::deserializeUnit(buf, offset, &ss1);
    offset += Deserializer::DeserializeUnit<uint64_t>::deserializeUnit(buf, offset, &uu1);

    EXPECT_EQ(ii, ii1);
    EXPECT_EQ(dd, dd1);
    EXPECT_EQ(ss, ss1);
    EXPECT_EQ(uu, uu1);

    buf.clear();
    Serializer::SerializeUnit<int>::serializeUnit(buf, &ii);
    Serializer::SerializeUnit<double>::serializeUnit(buf, &dd);

    offset = 0;
    offset += Deserializer::DeserializeUnit<short>::deserializeUnit(buf, offset, &ss1);
    offset += Deserializer::DeserializeUnit<int>::deserializeUnit(buf, offset, &ii1);
    EXPECT_NE(ii, ii1);
    EXPECT_NE(ss, ss1);
}

TEST(VectorLikeContainersTest, SerializerTests)
{
    using namespace Serialization;

    std::vector<int> vi{1,2,3,4,5};
    std::string buf;
    Serializer::SerializeUnit<std::vector<int>>::serializeUnit(buf, &vi);

    std::vector<int> vi1;
    Deserializer::DeserializeUnit<std::vector<int>>::deserializeUnit(buf, 0, &vi1);

    EXPECT_TRUE(areContainersEqual(vi, vi1));

    std::vector<double> vd{2.2,5.5, 33.3, 36.6};
    buf.clear();

    Serializer::SerializeUnit<std::vector<double>>::serializeUnit(buf, &vd);

    std::vector<double> vd1;
    Deserializer::DeserializeUnit<std::vector<double>>::deserializeUnit(buf, 0, &vd1);

    EXPECT_TRUE(areContainersEqual(vd, vd1));

    Deserializer::DeserializeUnit<std::vector<int>>::deserializeUnit(buf, 0, &vi1);
    EXPECT_FALSE(areContainersEqual(vi, vi1));

    std::list<char> lc{'a', 'b', 'z', 'd'};
    buf.clear();
    Serializer::SerializeUnit<std::list<char>>::serializeUnit(buf, &lc);

    std::list<char> lc1;
    Deserializer::DeserializeUnit<std::list<char>>::deserializeUnit(buf, 0, &lc1);
    EXPECT_TRUE(areContainersEqual(lc, lc1));

    buf.clear();
    std::deque<std::string> ds;
    ds.push_back("neko");
    ds.push_back("wanko");
    ds.push_back("chinko");
    ds.push_back("manko");
    Serializer::SerializeUnit<std::deque<std::string>>::serializeUnit(buf, &ds);

    std::deque<std::string> ds1;
    Deserializer::DeserializeUnit<std::deque<std::string>>::deserializeUnit(buf, 0, &ds1);
    EXPECT_TRUE(areContainersEqual(ds, ds1));

}

TEST(EmptyContainerTest, SerializerTest)
{
    using namespace Serialization;

    std::vector<char> vc;
    std::string buf;
    Serializer::SerializeUnit<std::vector<char>>::serializeUnit(buf, &vc);

    std::vector<char> vc1;
    Deserializer::DeserializeUnit<std::vector<char>>::deserializeUnit(buf, 0, &vc1);

    EXPECT_TRUE(areContainersEqual(vc, vc1));
    EXPECT_TRUE(vc1.empty());
}

TEST(StringTest, SerializerTest)
{
    using namespace Serialization;

    std::string buf;
    std::string neko("ababa");
    Serializer::SerializeUnit<std::string>::serializeUnit(buf, &neko);

    std::string neko1;
    Deserializer::DeserializeUnit<std::string>::deserializeUnit(buf, 0, &neko1);

    EXPECT_EQ(neko, neko1);

    std::string empty;
    buf.clear();
    Serializer::SerializeUnit<std::string>::serializeUnit(buf, &empty);

    std::string empty1;
    Deserializer::DeserializeUnit<std::string>::deserializeUnit(buf, 0, &empty1);

    EXPECT_EQ(empty, empty1);
}


TEST(SerializeAllTest, SerializerTest)
{
    using namespace Serialization;

    int i = 2;
    double d = 3.3;
    std::string s = "hello";
    std::vector<char> vc{'z','x','y'};

    std::string buf;
    Serializer::serializeAll(buf, &i, &s, &vc, &d);

    int i1;
    double d1;
    std::string s1;
    std::vector<char> vc1;

    Deserializer::deserializeAll(buf, 0, &i1, &s1, &vc1, &d1);
    EXPECT_EQ(i, i1);
    EXPECT_EQ(s, s1);
    EXPECT_TRUE(areContainersEqual(vc, vc1));
    EXPECT_EQ(d, d1);
}

TEST(UserTypesTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;

    User user;
    user.age = 22;
    user.name = "Vasya";
    user.hobbies = std::move(std::vector<std::string>{"swimming", "anime", "fishing"});

    BytesCount b = Serializer::SerializeUnit<User>::serializeUnit(buf, &user);
    EXPECT_EQ(b, sizeof(int) + sizeof(BytesCount) + 5 + sizeof(BytesCount) * 4 + 8 + 5 + 7);

    User user1;
    BytesCount b1 = Deserializer::DeserializeUnit<User>::deserializeUnit(buf, 0, &user1);
    EXPECT_EQ(b, b1);

    EXPECT_EQ(user.age, user1.age);
    EXPECT_EQ(user.name, user1.name);
    EXPECT_TRUE(areContainersEqual(user.hobbies, user1.hobbies));

    uint32_t ui = 32;
    char c = 'c';
    buf.clear();

    Serializer::serializeAll(buf, &ui, &c, &user);

    User user2;
    uint32_t ui1;
    char c1;
    Deserializer::deserializeAll(buf, 0, &ui1, &c1, &user2);

    EXPECT_EQ(ui, ui1);
    EXPECT_EQ(c, c1);
    EXPECT_EQ(user.age, user2.age);
    EXPECT_EQ(user.name, user2.name);
    EXPECT_TRUE(areContainersEqual(user.hobbies, user2.hobbies));
}

TEST(UserTypesMultupleSerializersTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;

    Cat cat;
    cat.age = 5;
    cat.legs = 4;
    cat.name = "Sugrob";
    cat.places_to_sleep = std::move(std::vector<std::string>{"chair", "laptop", "my head"});
    cat.setSerializerId((SerializerId)(Cat::AllSerializer));

    Serializer::serializeAll(buf, &cat);

    Cat cat1;
    cat1.setDeserializerId((SerializerId)(Cat::AllDeserializer));
    Deserializer::deserializeAll(buf, 0, &cat1);

    EXPECT_EQ(cat.age, cat1.age);
    EXPECT_EQ(cat.legs, cat1.legs);
    EXPECT_EQ(cat.name, cat1.name);
    EXPECT_TRUE(areContainersEqual(cat.places_to_sleep, cat1.places_to_sleep));

    Cat cat2;
    cat2.age = 5;
    cat2.legs = 4;
    cat2.name = "Sugrob";
    cat2.places_to_sleep = std::move(std::vector<std::string>{"chair", "laptop", "my head"});
    cat2.setSerializerId((SerializerId)(Cat::NameLegsSerializer));

    Serializer::serializeAll(buf, &cat2);

    Cat cat3;
    cat3.age = 0;
    cat3.setDeserializerId((SerializerId)(Cat::NameLegsDeserializer));
    Deserializer::deserializeAll(buf, 0, &cat3);

    EXPECT_NE(cat2.age, cat3.age);
    EXPECT_EQ(cat2.legs, cat3.legs);
    EXPECT_EQ(cat2.name, cat3.name);
    EXPECT_FALSE(areContainersEqual(cat2.places_to_sleep, cat3.places_to_sleep));
}


TEST(StdPairTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;

    std::pair<int, std::vector<std::string>> pivs{22, {"neko", "wanko", "kokos"}};
    Serializer::serializeAll(buf, &pivs);

    decltype(pivs) pivs1;
    Deserializer::deserializeAll(buf, 0, &pivs1);

    EXPECT_EQ(pivs.first, pivs1.first);
    EXPECT_TRUE(areContainersEqual(pivs.second, pivs1.second));
}


TEST(MapsTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;

    std::unordered_map<int, std::string> nekoMap{{1, "neko"}, {2, "wanko"}, {3, "manko"}};
    Serializer::serializeAll(buf, &nekoMap);

    std::unordered_map<int, std::string> nekoMap1;
    Deserializer::deserializeAll(buf, 0, &nekoMap1);

    EXPECT_TRUE(nekoMap.size() == nekoMap1.size());
    EXPECT_TRUE(nekoMap[1] == nekoMap1[1]);

    std::map<std::string, std::vector<std::string>> m{{"neko", {"nya", "mew", "meow"}}, {"wanko", {"wan", "gav", "gao"}}};
    buf.clear();
    Serializer::serializeAll(buf, &m);

    std::map<std::string, std::vector<std::string>> m1;
    Deserializer::deserializeAll(buf, 0, &m1);

    EXPECT_TRUE(m.size() == m1.size());
    EXPECT_TRUE(m.at("wanko") == m1.at("wanko"));

    // multimaps

    buf.clear();
    std::unordered_multimap<int, double> umm{{1,2.2}, {2, 3.3}, {1, 4.4}};
    Serializer::serializeAll(buf, &umm);

    std::unordered_multimap<int, double> umm1;
    Deserializer::deserializeAll(buf, 0, &umm1);

    EXPECT_TRUE(umm.size() == umm1.size());
}

TEST(SetsTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;
    std::set<int> s{1,2,3};

    Serializer::serializeAll(buf, &s);

    std::set<int> s1;
    Deserializer::deserializeAll(buf, 0, &s1);

    EXPECT_TRUE(areContainersEqual(s, s1));

    buf.clear();
    std::unordered_set<std::string> us{"hi", "i", "am", "vasya"};

    BytesCount b = Serializer::serializeAll(buf, &us);
    EXPECT_EQ(b, sizeof(BytesCount) * 5 + 2 + 1 + 2 + 5);

    std::unordered_set<std::string> us1;
    BytesCount b1 = Deserializer::deserializeAll(buf, 0, &us1);
    EXPECT_EQ(b, b1);

    // order may be not the same, so we don't check it
    EXPECT_TRUE(us.size() == us1.size());

    buf.clear();
    std::multiset<std::vector<std::string>> msvs{ {"neko", "wanko", "chinko", "manko"},
                                                  {"sugrob", "markiz", "sklirs"},
                                                  {"apple", "android", "shit"}};
    b = Serializer::serializeAll(buf, &msvs);
    EXPECT_EQ(b, sizeof(BytesCount) * 14 + 4 + 5 + 6 + 5 + 6 + 6 + 6 + 5 + 7 + 4);

    std::multiset<std::vector<std::string>> msvs1;
    b1 = Deserializer::deserializeAll(buf, 0, &msvs1);
    EXPECT_TRUE(msvs.size() == msvs1.size());
    EXPECT_EQ(b, b1);
}

TEST(ArraysTest, SerializerTest)
{
    using namespace Serialization;
    std::string buf;

    int* pi = new int[5];
    pi[0] = 2;
    pi[1] = 5;
    pi[2] = 6;
    pi[3] = 1;
    pi[4] = 9;

    double* pd = new double[4];
    pd[0] = 2.2;
    pd[1] = 6.2;
    pd[2] = 5.9543;
    pd[3] = 1.2;

    ArrayWrapper<int> aw1{pi, 5};
    ArrayWrapper<double> aw2{pd, 4};
    BytesCount b = Serializer::serializeAll(buf, &aw1, &aw2);
    EXPECT_EQ(b, sizeof(ElementsCount) * 2 + 5 * sizeof(int) + 4 * sizeof(double));

    ArrayWrapper<int> aw11;
    aw11.start = new int[5];
    ArrayWrapper<double> aw21;
    aw21.start = new double[4];

    BytesCount b1 = Deserializer::deserializeAll(buf, 0, &aw11, &aw21);

    EXPECT_EQ(b1, b);
    EXPECT_EQ(pi[2], aw11.start[2]);
    EXPECT_EQ(pd[3], aw21.start[3]);

    // std::array
    buf.clear();
    std::array<int, 10> arr{1,2,3,4,5,6,7,8,9,10};
    ArrayWrapper<int> aw3{arr.data(), 10};

    b = Serializer::serializeAll(buf, &aw3);
    EXPECT_EQ(b, sizeof(ElementsCount) + sizeof(int) * 10);

    std::array<int, 10> arr1;
    ArrayWrapper<int> aw31{arr1.data(), 10};
    b1 = Deserializer::deserializeAll(buf, 0, &aw31);
    EXPECT_EQ(b, b1);
    EXPECT_EQ(arr.size(), arr1.size());
    EXPECT_TRUE(areContainersEqual(arr, arr1));
}
