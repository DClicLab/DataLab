#include <CSensor.h> 
#include <string.h>
#include <map>
#include <memory>


// template <typename T>
// class Factory
// {
// public:
//     template <typename TDerived>
//     void registerType(const char* name)
//     {
//         static_assert(std::is_base_of<T, TDerived>::value, "Factory::registerType doesn't accept this type because doesn't derive from base class");
//         _createFuncs[name] = &createFunc<TDerived>;
//     }

//     T* create(const char* name) {
//         typename std::map<const char*,PCreateFunc>::const_iterator it = _createFuncs.find(name);
//         if (it != _createFuncs.end()) {
//             return it.second()();
//         }
//         return nullptr;
//     }

// private:
//     template <typename TDerived>
//     static T* createFunc()
//     {
//         return new TDerived();
//     }

//     typedef T* (*PCreateFunc)();
//     std::map<const char*,PCreateFunc> _createFuncs;
// };

