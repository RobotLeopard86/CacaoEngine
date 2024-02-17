#pragma once

//Allows conversion of an instance member function into a form that can be called like a static function
#define BIND_MEMBER_FUNC(func) std::bind(&func, this, std::placeholders::_1)

namespace Cacao {
	//Empty native data struct
	struct NativeData {};
}