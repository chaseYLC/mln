#pragma once

namespace MLN
{
	namespace Net
	{
		class Container;

		class Component
		{
		public:
			virtual bool initialize() = 0;
			virtual bool release() = 0;
			virtual void onUpdate(unsigned long elapse) {}

			Container* owner(){
				return _owner;
			}

		private:
			Container* _owner;
			friend class Container;
		};
	};//namespace Net
};//namespace MLN