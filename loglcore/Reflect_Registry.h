#pragma once
#include <map>
#include <string>
#include <algorithm>
namespace Game {
	template<typename T>
	struct _REFLECT_REGISTRY {
		_REFLECT_REGISTRY(const char* _name,T val,bool tolowered = false) {
			std::string name = _name;
			
			if (tolowered) {
				std::string lower_name;
				lower_name.resize(name.size());
				std::transform(name.begin(),name.end(),lower_name.begin(),::tolower);
				name = lower_name;
			}
			_registry_map[name] = val;
		}
		static std::map<std::string, T> _registry_map;
	};

	template<typename T>
	struct RefRegister {
		static bool find(const char* name,T& val) {
			auto item = _REFLECT_REGISTRY<T>::_registry_map.find(name);
			if (item != _REFLECT_REGISTRY<T>::_registry_map.end()) {
				val = item->second;
				return true;
			}
			return false;
		}
	};


}
#define _REFLECT_REGISTER_TOLOWER(Item,Type,Prefix) _REFLECT_REGISTRY<Type> _reflect_registry_##Prefix##Item##(""#Item,Prefix##Item,true);
#define _REFLECT_REGISTER(Item,Type,Prefix) _REFLECT_REGISTRY<Type> _reflect_registry_##Prefix##Item##(""#Item,Prefix##Item);
#define _REFLECT_REGISTER_NOPREFIX(Item,Type,Namespace) _REFLECT_REGISTRY<Type> _reflect_registry_##Item##(""#Item,Namespace##Item);
#define _SETUP_REFLECTION(Type) std::map<std::string, Type> _REFLECT_REGISTRY<Type>::_registry_map;