#pragma once

namespace Citrus {
    //Base texture type
    class Texture {
    public:
        //Use this texture
        virtual void Bind() {}
        //Don't use this texture
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
    };
}