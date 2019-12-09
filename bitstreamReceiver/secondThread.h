#pragma once
class secondThread
{
public:
	secondThread();
	~secondThread();

	int Start2();
	int Kill2();

	bool IsRunning2();

	virtual void* Thread2() = 0;

private:
	static UINT __stdcall TheThread(void* param);

private:
	bool running;
	UINT threadid;
	HANDLE threadhandle;
};

