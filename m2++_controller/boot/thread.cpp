#include "thread.h"

void __stdcall boot::thread_ext::setup()
{
	uvirt;
	SetConsoleTitleA(randstr(20));
	
	util::c_log::Instance().setup();
	boot::c_conf::Instance().setup();	
	
	vmend;
}

void boot::c_thread::setup()
{
	uvirt;
	this->core = ([this]
	{
		boot::thread_ext::setup();
	});
	std::thread(this->core).join();
	util::c_log::Instance().duo(XorStr("[ boot executed ]\n"));
	vmend;
}

void boot::c_thread::work()
{
	while (true)
	{
		uvirt;
		const auto time = GetTickCount64();
		for (auto&& obj : this->pool)
		{
			if (obj->last_exec + obj->interval < time) continue;
			obj->last_exec = time + obj->interval;
			try 
			{
				obj->func();
			}
			catch (std::exception &e)
			{
				util::c_log::Instance().duo(XorStr("[ function %04x has failed => %s ]\n", &obj->func, e.what()));
			}
		}
		std::this_thread::sleep_for(10ms);
		vmend;
	}
}

bool boot::c_thread::add(thread_strc::s_thread_i* t)
{
	uvirt;
	this->pool.push_back(t);
	vmend;
	return true;	
}
