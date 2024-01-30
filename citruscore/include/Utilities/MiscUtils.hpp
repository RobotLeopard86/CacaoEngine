#pragma once

//Allows conversion of an instance member function into a form that can be called like a static function
#define BIND_MEMBER_FUNC(f) std::bind(&f, this, std::placeholders::_1)