#include "console.h"
#include <ctime>

COORD getXY()							//通过WindowsAPI函数获取光标的位置
{
	CONSOLE_SCREEN_BUFFER_INFO pBuffer;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pBuffer);
	//利用标准输出句柄获得光标坐标信息

	return COORD{ pBuffer.dwCursorPosition.X, pBuffer.dwCursorPosition.Y };
	//封装为表示坐标的COORD结构
}

COORD getScrnInfo()										//获取控制台窗口缓冲区大小
{
	HANDLE hStd = GetStdHandle(STD_OUTPUT_HANDLE);		//获得标准输出设备句柄
	CONSOLE_SCREEN_BUFFER_INFO scBufInf;				//定义一个窗口缓冲区信息结构体

	GetConsoleScreenBufferInfo(hStd, &scBufInf);		//获取窗口缓冲区信息

	return scBufInf.dwSize;								//返回窗口缓冲区大小
}

void moveXY(COORD pstn)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pstn);
	//通过标准输出句柄控制光标的位置
}

void clearDisplay(COORD firstPst, COORD lastPst)		//清除部分屏幕内容，从firstPst坐标到lastPst坐标之间的内容
{
	int yDValue(lastPst.Y - firstPst.Y);					//记录首末位置纵坐标差值，控制迭代次数

	COORD size(getScrnInfo());								//记录目前控制台缓冲区大小

	moveXY(firstPst);							//移动光标到首位置
	for (int y(0); y <= yDValue; y++)			//一层循环控制清除行数
	{
		for (int x(firstPst.X); x <= size.X; x++)			//二层循环避免重复清除
		{
			std::cout << ' ';						//输出一个空格来覆盖原内容，达到清除效果
			int px;									//记录光标当前位置的横坐标
			if (x != size.X)
				px = x + 1;
			else
				px = 0;
			if (y == yDValue && px == lastPst.X)		//与光标末位置作对比，达到末位置即退出循环
				break;
		}
	}
	moveXY(firstPst);
}

void loading(float process,const char* desc1,const char* desc2)									//等待界面，模拟动态进度条过程
{
	int n = (int)(process * 100.f);
	n = n > 100 ? 100 : n;
	
	const char* anim = "-\\|/";
	static int counter = 0;

	int m = 50;

	std::cout << desc1 << "|";

	for (int i = n * m / 100; i > 0; i--)
		std::cout << "█";
	for (int i = m - n * m / 100; i > 0; i--) {
		std::cout << " ";
	}
	std::cout << "|";
	printf("%2.2f %% [%c]  %s", process * 100.f , anim[counter] , desc2);//输出百分比进度条
	counter = (counter + 1) % strlen(anim);
}

Console& Console::FlushLog() {
	this->cursor = getXY();
	return *this;
}

Console& Console::ClearLog() {
	COORD currXY = getXY();
	clearDisplay(this->cursor, currXY);
	moveXY(this->cursor);
	return *this;
}

Console& Console::LogProcess(const char* desc1,float process, const char* desc2) {
	loading(process, desc1, desc2);
	return *this;
}

Console& Console::LineSwitch(size_t num) {
	for(size_t i = 0;i != num;i++)
		std::cout << std::endl;
	return *this;
}

Console::Console() {
	HANDLE hStd(GetStdHandle(STD_OUTPUT_HANDLE));
	CONSOLE_CURSOR_INFO  cInfo;
	GetConsoleCursorInfo(hStd, &cInfo);			//获取光标信息的句柄
	cInfo.bVisible = false;						//修改光标可见性
	SetConsoleCursorInfo(hStd, &cInfo);			//设置光标不可见

}

