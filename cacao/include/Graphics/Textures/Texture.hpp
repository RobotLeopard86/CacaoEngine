#pragma once

namespace Cacao {
    //Base texture type
    class Texture {
    public:
        //Attach this texture to the specified slot
        virtual void Bind(int slot) {}
        //Detach this texture
        virtual void Unbind() {}
        //Compile texture to be used later
        virtual void Compile() {}
        //Delete texture when no longer needed
        virtual void Release() {}

        //Is texture compiled?
        bool IsCompiled() { return compiled; }

        //Is texture bound?
        bool IsBound() { return bound; }
    protected:
        bool compiled;
        bool bound;
		int currentSlot;
    };
}