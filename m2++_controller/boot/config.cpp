#include "config.h"
void boot::c_conf::setup()
{
	uvirt;
	const auto did_load = this->load();
	dbglog(XorStr("[ base config load result > %s ]\n"), did_load ? XorStr("ok") : XorStr("not available"));
	if (!did_load)
	{
		//set data dir
		this->base_config = {};
		
		TCHAR buffer[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, buffer, MAX_PATH);

		this->base_config.data_dir = buffer;

		const auto last_dir_slash = this->base_config.data_dir.find_last_of('\\');
		if (last_dir_slash != std::string::npos) this->base_config.data_dir.erase(last_dir_slash, this->base_config.data_dir.size());
	}
	dbglogw(XorStrW(L"[ data dir: %s ]\n"), this->base_config.data_dir.c_str());
	//always rand
	this->base_config.ipc_comkey = randstr(64);
	
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) dbglog(XorStr("[ failed opening WSAStartup ]\n"));
	
	while (true)
	{
		this->base_config.ipc_port = std::to_string(randint(9999));

		SOCKADDR_IN clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(XorStr("127.0.0.1"));
		clientService.sin_port = htons(std::stoi(this->base_config.ipc_port));
		SOCKET m_socket;
		m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET) 
		{
			dbglog(XorStr("[ opening socket [%s] failed => %ld ]\n"), this->base_config.ipc_port.c_str(), WSAGetLastError());
			continue;
		}
		if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) != SOCKET_ERROR)
		{
			dbglog(XorStr("[ socket [%s] already used => %ld ]\n"), this->base_config.ipc_port.c_str(), WSAGetLastError());
			continue;
		}
		closesocket(m_socket);
		dbglog(XorStr("[ found available ipc port => %s ]\n"), this->base_config.ipc_port.c_str());
		break;
	}
	dbglog(XorStr("[ ipc key : %s ]\n"), this->base_config.ipc_comkey.c_str());
	dbglog(XorStr("[ ipc port: %s ]\n"), this->base_config.ipc_port.c_str());
	
	this->save();
	
	vmend;
}

bool boot::c_conf::save()
{
	std::ofstream	bconf(XorStr("M2++CONTROLLER_BASE.CFG"));
	if (!bconf.is_open()) return FALSE;
	
	auto j = nlohmann::json();
	nlohmann::to_json(j, this->base_config);

	bconf << j << "\n";

	bconf.close();

	return TRUE;
}

bool boot::c_conf::load()
{
	std::ifstream	bconf(XorStr("M2++CONTROLLER_BASE.CFG"));
	if (!bconf.is_open()) return FALSE;
	
	std::string		filebuf;
	std::getline(bconf, filebuf);
	if (filebuf.empty()) return FALSE;
	
	auto jparse = nlohmann::json::parse(filebuf);
	this->base_config = jparse.get<boot::conf_json::c_config>();

	bconf.close();

	return TRUE;
}
