#pragma once

#include <kaguya/kaguya.hpp>

namespace obe
{
    namespace Bindings
    {
        /**
        * \brief Bindings to SFML related classes and functions
        */
        namespace SFMLBindings
        {
            void LoadSfColor(kaguya::State* lua);
            void LoadSfDrawable(kaguya::State* lua);
            void LoadSfFont(kaguya::State* lua);
            void LoadSfShape(kaguya::State* lua);
            void LoadSfSprite(kaguya::State* lua);
            void LoadSfText(kaguya::State* lua);
            void LoadSfTransformable(kaguya::State* lua);
            void LoadSfVector(kaguya::State* lua);
        }
    }
}