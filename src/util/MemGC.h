#pragma once
#include <cassert>
#include <cstdio>
#include <thread>
#include <mutex>
#include <list>

class Deleter
{
public:
	virtual ~Deleter() {};
public:
	int generation;
};

template<typename T>
class DeleterImp: public Deleter
{
public:
	DeleterImp(T* _ptr)
		:ptr(_ptr)
	{
		assert(_ptr!=NULL);
	}
	virtual ~DeleterImp()
	{
		delete ptr;
	}
private:
	T* ptr;
};

class MemGC
{
	typedef std::list<Deleter*> Buffer;
public:
	static MemGC& instance();
	~MemGC(void);
	void addGarbage(Deleter* garbage);
	void doGC();
	void stop();
private:
	MemGC(int, long);
	std::thread del_thread;
	std::mutex del_mutex;
	Buffer buffer;
	std::chrono::seconds interval;
	bool stop_flag;
	int max_gen;
private:
	static std::mutex instance_mutex;
};

