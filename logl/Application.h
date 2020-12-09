#pragma once

class Application {
public:
	bool initialize();
	void update();
	void finalize();
};

inline Application gApp;
