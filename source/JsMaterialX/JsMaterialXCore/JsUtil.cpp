#include <MaterialXCore/Util.h>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

extern "C"
{
    EMSCRIPTEN_BINDINGS(util)
    {
        ems::function("getVersionString", &mx::getVersionString);

        // The following function throw: Cannot call {function name} due to unbound types: XXXXX
        ems::function("getVersionIntegers", ems::optional_override([]() {
                     auto version = mx::getVersionIntegers();
                     return ems::val::array((int *)&version, (int *)&version + 3);
                 }));

        ems::function("createValidName", &mx::createValidName); // arg0 === {std::string}, arg1 === {unicode representing character}
        ems::function("isValidName", &mx::isValidName);
        ems::function("incrementName", &mx::incrementName);

        ems::function("splitNamePath", &mx::splitNamePath);
        ems::function("createNamePath", &mx::createNamePath);
        ems::function("parentNamePath", &mx::parentNamePath);
    }
}