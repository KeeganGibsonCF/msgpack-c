#include <msgpack.hpp>
#include <gtest/gtest.h>

struct myclass {
    myclass() : num(0), str("default") { }

    myclass(int num, const std::string& str) :
        num(num), str(str) { }

    ~myclass() { }

    int num;
    std::string str;
    std::vector<double> vec;
    std::map<std::string, std::vector<char> > map;

    MSGPACK_DEFINE(num, str, vec, map);

    bool operator==(const myclass& o) const
    {
        return num == o.num && str == o.str && vec == o.vec && map == o.map;
    }
};

std::ostream& operator<<(std::ostream& o, const myclass& m)
{
    return o << "myclass("<<m.num<<",\""<<m.str<<"\")";
}


TEST(object, convert)
{
    myclass m1(1, "custom");

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, m1);

    msgpack::unpacked ret;
    msgpack::unpack(ret, sbuf.data(), sbuf.size());

    myclass m2;
    ret.get().convert(&m2);

    EXPECT_EQ(m1, m2);
}


TEST(object, as)
{
    myclass m1(1, "custom");

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, m1);

    msgpack::unpacked ret;
    msgpack::unpack(ret, sbuf.data(), sbuf.size());

    EXPECT_EQ(m1, ret.get().as<myclass>());
}

TEST(object, cross_zone_copy)
{
    myclass m1(1, "custom");
    m1.vec.push_back(1.0);
    m1.vec.push_back(0.1);
    std::vector<char> vc;
    vc.push_back('t');
    vc.push_back('w');
    vc.push_back('o');
    m1.map["one"] = vc;

    msgpack::zone z1;
    msgpack::object::with_zone obj1(z1);

    {
        msgpack::zone z2;
        msgpack::object::with_zone obj2(z2);
        obj2 << m1;

        obj1 << obj2;

        EXPECT_EQ(obj1.via.array.ptr[2].via.array.ptr[0].via.dec, 1.0);
        EXPECT_EQ(obj1.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr[0], 'o');
        EXPECT_EQ(obj1.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr[0], 't');
        EXPECT_NE(
            obj1.via.array.ptr[2].via.array.ptr,
            obj2.via.array.ptr[2].via.array.ptr);
        EXPECT_NE(
            obj1.via.array.ptr[3].via.map.ptr,
            obj2.via.array.ptr[3].via.map.ptr);
        EXPECT_NE(
            obj1.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr,
            obj2.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr);
        EXPECT_NE(
            obj1.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr,
            obj2.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr);
    }

    EXPECT_EQ(m1, obj1.as<myclass>());
}

TEST(object, cross_zone_copy_construct)
{
    myclass m1(1, "custom");
    m1.vec.push_back(1.0);
    m1.vec.push_back(0.1);
    std::vector<char> vc;
    vc.push_back('t');
    vc.push_back('w');
    vc.push_back('o');
    m1.map["one"] = vc;

    msgpack::zone z1;
    msgpack::zone z2;
    msgpack::object::with_zone obj2(z2);
    obj2 << m1;

    msgpack::object obj1(obj2, z1);

    EXPECT_EQ(obj1.via.array.ptr[2].via.array.ptr[0].via.dec, 1.0);
    EXPECT_EQ(obj1.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr[0], 'o');
    EXPECT_EQ(obj1.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr[0], 't');
    EXPECT_NE(
        obj1.via.array.ptr[2].via.array.ptr,
        obj2.via.array.ptr[2].via.array.ptr);
    EXPECT_NE(
        obj1.via.array.ptr[3].via.map.ptr,
        obj2.via.array.ptr[3].via.map.ptr);
    EXPECT_NE(
        obj1.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr,
        obj2.via.array.ptr[3].via.map.ptr[0].key.via.str.ptr);
    EXPECT_NE(
        obj1.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr,
        obj2.via.array.ptr[3].via.map.ptr[0].val.via.bin.ptr);
    EXPECT_EQ(m1, obj1.as<myclass>());
}

TEST(object, cross_zone_copy_ext)
{
    msgpack::zone z1;
    msgpack::zone z2;
    msgpack::object::with_zone obj1(z1);

    obj1.type = msgpack::type::EXT;
    char* ptr = static_cast<char*>(obj1.zone.allocate_align(2));
    ptr[0] = 1;
    ptr[1] = 2;
    obj1.via.ext.ptr = ptr;
    obj1.via.ext.size = 1;

    msgpack::object::with_zone obj2(z2);
    obj2 << obj1;
    EXPECT_EQ(obj2.via.ext.size, 1);
    EXPECT_EQ(obj2.via.ext.ptr[0], 1);
    EXPECT_EQ(obj2.via.ext.ptr[1], 2);
    EXPECT_NE(
        obj1.via.ext.ptr,
        obj2.via.ext.ptr);
}

TEST(object, cross_zone_copy_construct_ext)
{
    msgpack::zone z1;
    msgpack::zone z2;
    msgpack::object::with_zone obj1(z1);

    obj1.type = msgpack::type::EXT;
    char* ptr = static_cast<char*>(obj1.zone.allocate_align(2));
    ptr[0] = 1;
    ptr[1] = 2;
    obj1.via.ext.ptr = ptr;
    obj1.via.ext.size = 1;

    msgpack::object obj2(obj1, z2);
    EXPECT_EQ(obj2.via.ext.size, 1);
    EXPECT_EQ(obj2.via.ext.ptr[0], 1);
    EXPECT_EQ(obj2.via.ext.ptr[1], 2);
    EXPECT_NE(
        obj1.via.ext.ptr,
        obj2.via.ext.ptr);
}

TEST(object, print)
{
    msgpack::object obj;
    std::cout << obj << std::endl;
}


TEST(object, is_nil)
{
    msgpack::object obj;
    EXPECT_TRUE(obj.is_nil());
}


TEST(object, type_error)
{
    msgpack::object obj(1);
    EXPECT_THROW(obj.as<std::string>(), msgpack::type_error);
    EXPECT_THROW(obj.as<std::vector<int> >(), msgpack::type_error);
    EXPECT_EQ(1, obj.as<int>());
    EXPECT_EQ(1, obj.as<short>());
    EXPECT_EQ(1u, obj.as<unsigned int>());
    EXPECT_EQ(1u, obj.as<unsigned long>());
}


TEST(object, equal_primitive)
{
    msgpack::object obj_nil;
    EXPECT_EQ(obj_nil, msgpack::object());

    msgpack::object obj_int(1);
    EXPECT_EQ(obj_int, msgpack::object(1));
    EXPECT_EQ(obj_int, 1);

    msgpack::object obj_double(1.2);
    EXPECT_EQ(obj_double, msgpack::object(1.2));
    EXPECT_EQ(obj_double, 1.2);

    msgpack::object obj_bool(true);
    EXPECT_EQ(obj_bool, msgpack::object(true));
    EXPECT_EQ(obj_bool, true);
}


TEST(object, construct_primitive)
{
    msgpack::object obj_nil;
    EXPECT_EQ(msgpack::type::NIL, obj_nil.type);

    msgpack::object obj_uint(1);
    EXPECT_EQ(msgpack::type::POSITIVE_INTEGER, obj_uint.type);
    EXPECT_EQ(1u, obj_uint.via.u64);

    msgpack::object obj_int(-1);
    EXPECT_EQ(msgpack::type::NEGATIVE_INTEGER, obj_int.type);
    EXPECT_EQ(-1, obj_int.via.i64);

    msgpack::object obj_double(1.2);
    EXPECT_EQ(msgpack::type::DOUBLE, obj_double.type);
    EXPECT_EQ(1.2, obj_double.via.dec);

    msgpack::object obj_bool(true);
    EXPECT_EQ(msgpack::type::BOOLEAN, obj_bool.type);
    EXPECT_EQ(true, obj_bool.via.boolean);
}
