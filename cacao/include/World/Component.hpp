#pragma once

namespace Cacao {
	//A component on an entity
	class Component {
	public:
		const bool IsActive() { return active; }
		virtual void SetActive(bool value) { active = value; }
	protected:
		//Is this component active?
		//Protected so subclasses can implement logic regarding this
		//The value of this boolean should be ignored if the entity active state is false
		bool active;
	};
}