#pragma once

#include <vector>

//Basic utilities for a tree data structure because SOMEONE (*cough cough* STL *cough cough*) doesn't provide one
namespace Cacao {
	//Item in a tree
	template<typename T>
	class TreeItem {
	public:
		std::vector<TreeItem<T>> children;

		T& operator->(){
			return node;
		}
	private:
		T node;
	};

	//A tree class with children, useful if there isn't one root node in your tree
	template<typename T>
	class Tree {
	public:
		std::vector<TreeItem<T>> children;
	};
}