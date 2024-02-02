#pragma once

namespace Citrus {
	template <typename T>
	class Flushable {
	public:
		explicit Flushable(T& obj) : originalObject(obj), mod(obj) {}

		//Access the local copy
		T* operator->() {
			return &mod;
		}

		//Flush changes to the original object
		void Flush() {
			originalObject = mod;
		}
	private:
		T& originalObject;
		T mod; //Local copy to work on
	};
}