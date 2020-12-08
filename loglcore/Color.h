#pragma once
#include "Vector.h"

namespace Game {
	using Color = Vector4;
	namespace ConstColor {
		extern Color Black ;
		extern Color White ;
		extern Color Red ;
		extern Color Blue ;
		extern Color Green ;
		extern Color Yellow ;
		extern Color Purple ;
		extern Color Orange ;


		bool GetColorByName(const char* name,Color& target);
		bool PraseColor(const char* colorStr,Color& target);
	};
}