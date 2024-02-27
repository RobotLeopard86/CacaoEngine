#pragma once

namespace Cacao {
	//A component on an entity
	class Component {
	public:
		const bool IsActive() const { return active; }
		virtual void SetActive(bool value) { active = value; }
	protected:
		//Is this component active?
		//The value of this boolean should be ignored if the entity active state is false
		bool active;
	};
}