#pragma once

namespace Cacao {
	//Utility class for storing temporary data and writing it back to an original object later
	template<typename T>
	class Flushable {
	public:
		explicit Flushable(T& obj) : originalObject(obj), mod(obj) {}

		//Access the local copy
		T* operator->() {
			return &mod;
		}

		//Overwrite the local copy with new data
		void operator=(T newDat){
			mod = newDat;
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