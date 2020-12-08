#include "Color.h"
#include <map>
#include "Reflect_Registry.h"

namespace Game {
	_SETUP_REFLECTION(Color);

#define DEFINE_COLOR(ColorName,R,G,B) Color ConstColor::##ColorName = Vector4(R,G,B,1.);\
_REFLECT_REGISTER_NOPREFIX(ColorName,Color,ConstColor::);

	DEFINE_COLOR(Black, 0., 0., 0.);
	DEFINE_COLOR(White, 1., 1., 1.);
	DEFINE_COLOR(Red, 1., 0., 0.);
	DEFINE_COLOR(Blue, 0., 0., 1.);
	DEFINE_COLOR(Green, 0., 1., 0.);
	DEFINE_COLOR(Yellow, 0., 1., 1.);
	DEFINE_COLOR(Purple, 1., 0., 1.);
	DEFINE_COLOR(Orange, 1., 1., 0.);

	bool ConstColor::GetColorByName(const char* name,Color& target) {
		std::string colorname = name;
		colorname[0] = toupper(colorname[0]);

		return RefRegister<Color>::find(colorname.c_str(),target);
	}

	int getHex(char num) {
		if (num >= '0' && num <= '9') return num - '0';
		num = tolower(num);
		if (num >= 'a' && num <= 'f') return num - 'a' + 10;
		return -1;
	}

	bool ConstColor::PraseColor(const char* colorstr,Color& target) {
		if (colorstr[0] == '#') colorstr++;
		for (int i = 0; i != 4; i++) {
			char up, down;
			if (up = getHex(colorstr[i * 2]) < 0) return false;
			if (down = getHex(colorstr[i * 2 + 1]) < 0) return false;
			float num = (up * 16 + down) / (float)256.;
			target.raw[i] = num;
		}
		return true;
	}
}