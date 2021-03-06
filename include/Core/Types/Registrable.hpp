#pragma once

#include <kaguya/kaguya.hpp>

#include <Script/GlobalState.hpp>

namespace obe
{
    namespace Types
    {
        template <class T> 
        class Registrable
        {
        public:
            void reg(const std::string& id)
            {
                Script::ScriptEngine[id] = static_cast<T*>(this);
            }
        };
    }
}