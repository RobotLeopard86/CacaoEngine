#pragma once

//This file serves to contain Citrus Engine utilities for both the engine and the client

//Allows conversion of an instance member into a bindable function
#define BIND_FUNC(f) std::bind(&f, this, std::placeholders::_1)