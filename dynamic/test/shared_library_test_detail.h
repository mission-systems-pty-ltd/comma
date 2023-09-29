#pragma once
#include <string>

namespace comma { namespace dynamic { namespace test {

class simple{
    std::string name{"hello world"};
};

class point{
    public:
        point() : x(0), y(0), z(0) {}
        point(float x, float y, float z) : x(x), y(y), z(z) {}
        float x;
        float y;
        float z;
};

class polymorphic_point {
public:
    polymorphic_point() : x(0), y(0), z(0) {}
    polymorphic_point(float x, float y, float z) : x(x), y(y), z(z) {}
    virtual ~polymorphic_point() {}
    virtual float get_x() const { return x; }
    virtual float get_y() const { return y; }
    virtual float get_z() const { return z; }

private:
    float x;
    float y;
    float z;
};

} } } // namespace comma { namespace dynamic { namespace test {
