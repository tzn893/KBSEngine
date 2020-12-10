#pragma once
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif


class Application {
public:
	bool initialize();
	void update();
	void finalize();
};

inline Application gApp;
