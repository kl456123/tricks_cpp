#include <iostream>
#include <string>

using namespace std;

enum class DataType{
    INT,
    UINT,
    FLOAT,
    DOUBLE
};

template<typename T>
struct DataTypeToEnum{
    DataType type;
    string value;
};


#define DEFINE_TYPE_TRAITS(T, type)         \
    template <>                             \
    struct DataTypeToEnum<T>{               \
        DataType type=DataType::type;      \
        string value=#T;                  \
    };

DEFINE_TYPE_TRAITS(double, DOUBLE)
DEFINE_TYPE_TRAITS(int, INT)


int main(){
    auto tmp = DataTypeToEnum<double>();
    cout<< tmp.value<<endl;
    return 0;
}
