#pragma once
#include <ctime>
#include <cstdio>
#include <string>
#include <chrono>

class Timer
{
private:
    std::string name;
    std::chrono::high_resolution_clock::time_point t1, t2;

public:
	Timer()
	{
	}

	Timer(std::string _name)
		: name(_name)
	{
	}

	void start()
	{
        using namespace std::chrono;
        t1 = high_resolution_clock::now();
	}

	void stop()
	{
        using namespace std::chrono;
        t2 = high_resolution_clock::now();
	}

	void show()
	{
		double elapsedTime = getPassedMilliseconds();
		printf("%s used: %.0lf ms\n", name.c_str(), elapsedTime);
	}

	double getPassedMilliseconds()
	{
        using namespace std::chrono;
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        return time_span.count() * 1000;
	}
};
