#pragma once

#include <boost/unordered_map.hpp>
#include "component.h"

namespace MLN
{
	namespace Net
	{
		class Container
		{
		public:
			~Container()
			{
				for (auto it : m_components){
					delete it.second;
				}
				m_components.clear();
			}

			template< typename T > T* addComponent();
			template< typename T > T* getComponent();
			template< typename T > bool removeComponent();

		protected:
			boost::unordered_map< size_t, Component* > m_components;
		};

		template< typename T >
		T* Container::addComponent()
		{
			T* component = nullptr;

			size_t typeCode = typeid(T).hash_code();
			auto it = m_components.find(typeCode);

			if (m_components.end() == it)
			{
				component = new T;
				component->_owner = this;
			}

			if (false == component->initialize())
			{
				delete component;
				return nullptr;
			}

			m_components.insert(std::make_pair(typeCode, component));
			return component;
		}

		template< typename T >
		T* Container::getComponent()
		{
			size_t typeCode = typeid(T).hash_code();
			auto it = m_components.find(typeCode);
			if (m_components.end() == it){
				return nullptr;
			}
			return static_cast<T*>(it->second);
		}

		template< typename T >
		bool Container::removeComponent()
		{
			size_t typeCode = typeid(T).hash_code();
			auto it = m_components.find(typeCode);
			if (m_components.end() == it){
				return false;
			}

			it->second->release();
			delete it->second;
			m_components.erase(it);

			return true;
		}
	};//namespace Net

};//namespace MLN
