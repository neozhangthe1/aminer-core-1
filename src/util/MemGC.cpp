#include "MemGC.h"

std::mutex MemGC::instance_mutex;

MemGC::MemGC(int max, long gc_interval)
	:del_thread(&MemGC::doGC, this),
	stop_flag(false),
	interval(gc_interval),
	max_gen(max)
{
}

MemGC& MemGC::instance()
{
	static MemGC* ptr = NULL;
	if(ptr==NULL)
	{
		std::lock_guard<std::mutex> lock(instance_mutex);
		if(ptr == NULL)
		{
			//read from configure file here
			int max_gen = 2;
			long interval = 50;
			ptr = new MemGC(max_gen, interval);
		}
	}
	return *ptr;
}

MemGC::~MemGC(void)
{
	//do nothing
}

void MemGC::addGarbage(Deleter* garbage)
{
	assert(garbage != NULL);
	garbage->generation = 0;
	std::lock_guard<std::mutex> lock(del_mutex);
	this->buffer.push_back(garbage);
}

void MemGC::doGC()
{
	printf("Start GC...\n");
	int left = 0;
	while(!stop_flag || left)
	{
		{
            std::lock_guard<std::mutex> lock(del_mutex);
			for (Deleter* deleter : buffer)
			{
				++(deleter->generation);
			}

			Deleter* deleter;
			if(!buffer.empty())
			{
				while(!buffer.empty() && (deleter = buffer.front())->generation == this->max_gen)
				{
					delete deleter;
					buffer.pop_front();
				}
			}
			left = buffer.size();
		}
		std::this_thread::sleep_for(this->interval);
	}
	printf("GC stoped...size = %d\n", buffer.size());
}

void MemGC::stop()
{
	stop_flag = true;
}
