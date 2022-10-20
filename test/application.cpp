#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <oocmd.hpp>

#include <iostream>

namespace oocmd::test {

using namespace oocmd;

Application parse(ConfigObject& e, std::vector<std::string>& v) {
    char** args = new char*[v.size()];
    for(size_t i = 0; i < v.size(); i++) {
        args[i] = v[i].data();
    }

    auto app = Application(e, (int)v.size(), args);
    delete[] args;
    return app;
}

class A : public ConfigObject {
public:
    bool x_ = false;

    A() : ConfigObject("A", "Entity A") {
        param("x", x_);
    }
};

template<typename T>
class Test : public ConfigObject {
public:
    bool                     bool_param_ = false;
    int                      int_param_ = 0;
    unsigned int             uint_param_ = 0U;
    uint64_t                 bytes_param_ = 0ULL;
    float                    float_param_ = 0.0f;
    double                   double_param_ = 0.0;
    std::string              string_param_;
    std::vector<std::string> stringlist_param_;
    T                        object_param_;

    Test() : ConfigObject("Test", "A test executable") {
        param("bool", bool_param_);
        param("int", int_param_);
        param("uint", uint_param_);
        param("bytes", bytes_param_);
        param("float", float_param_);
        param("double", double_param_);
        param("string", string_param_);
        param("stringlist", stringlist_param_);
        param("object", object_param_);
    }
};

TEST_SUITE("application") {
    TEST_CASE("Command-line defaults") {
        std::vector<std::string> args = { "<PATH>"};
        Test<A> a;
        auto app = parse(a, args);

        REQUIRE(app.good());
        CHECK(!a.bool_param_);
        CHECK(a.int_param_ == 0);
        CHECK(a.uint_param_ == 0U);
        CHECK(a.bytes_param_ == uint64_t(0));
        CHECK(a.float_param_ == 0.0f);
        CHECK(a.double_param_ == 0.0);
        CHECK(a.string_param_ == "");
        CHECK(a.stringlist_param_.empty());
        CHECK(!a.object_param_.x_);
        CHECK(app.args().empty());
    }

    TEST_CASE("Command-line configuration, differentiate between params and free arguments") {
        std::vector<std::string> args = { "<PATH>", "--bool",  "FREE1", "--uint", "5", "FREE2" };
        Test<A> a;
        auto app = parse(a, args);

        REQUIRE(app.good());
        CHECK(a.bool_param_);
        CHECK(a.uint_param_ == 5U);
        CHECK(app.args()[0] == "FREE1");
        CHECK(app.args()[1] == "FREE2");
    }

    TEST_CASE("Command-line configuration") {
        std::vector<std::string> args = { "<PATH>", "--bool", "--int=-5", "--uint", "5", "--bytes", "1Ki", "--float=-.5", "--double", "777.77", "--string", "test", "--stringlist", "X", "--stringlist=Y", "--object.x", "FREE" };
        Test<A> a;
        auto app = parse(a, args);

        REQUIRE(app.good());
        CHECK(a.bool_param_);
        CHECK(a.int_param_ == -5);
        CHECK(a.uint_param_ == 5U);
        CHECK(a.bytes_param_ == 1024ULL);
        CHECK(a.float_param_ == -0.5f);
        CHECK(a.double_param_ == 777.77);
        CHECK(a.string_param_ == "test");
        CHECK(a.stringlist_param_.size() == 2);
        CHECK(a.stringlist_param_[0] == "X");
        CHECK(a.stringlist_param_[1] == "Y");
        CHECK(a.object_param_.x_);
        CHECK(app.args()[0] == "FREE");
    }

    TEST_CASE("Command-line configuration, alternative syntax") {
        std::vector<std::string> args = { "<PATH>", "--bool=1", "--int=-5", "--uint=5", "--bytes=1Ki", "--float=-.5", "--double=777.77", "--string=test", "--stringlist=X", "--stringlist=Y", "--object.x", "FREE" };
        Test<A> a;
        auto app = parse(a, args);

        REQUIRE(app.good());
        CHECK(a.bool_param_);
        CHECK(a.int_param_ == -5);
        CHECK(a.uint_param_ == 5U);
        CHECK(a.bytes_param_ == 1024ULL);
        CHECK(a.float_param_ == -0.5f);
        CHECK(a.double_param_ == 777.77);
        CHECK(a.string_param_ == "test");
        CHECK(a.stringlist_param_.size() == 2);
        CHECK(a.stringlist_param_[0] == "X");
        CHECK(a.stringlist_param_[1] == "Y");
        CHECK(a.object_param_.x_);
        CHECK(app.args()[0] == "FREE");
    }
}

}
