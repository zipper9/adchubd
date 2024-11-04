/*
 * Copyright (C) 2006-2018 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <adchpp/adchpp.h>
#include <adchpp/common.h>

#include <adchpp/File.h>
#include <adchpp/LogManager.h>
#include <adchpp/Util.h>
#include <adchpp/version.h>

#include "adchppd.h"

#include <locale.h>

using namespace adchpp;
using std::string;

#define PRINTERROR(func) \
	fprintf(stderr, func " failed: code %lu: %s\n", GetLastError(), \
	        Util::translateError(GetLastError()).c_str())

bool asService = true;
static const TCHAR* serviceName = _T("adchubd");
static std::shared_ptr<Core> core;
static string configPath;

static void installService()
{
	SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (scm == NULL)
	{
		PRINTERROR("OpenSCManager");
		return;
	}

	string cmdLine = '"' + Util::getAppName() + "\" -c \"" + configPath + "\\\" -d \"" + string(serviceName) + "\"";
	SC_HANDLE service = CreateService(scm, serviceName, _T("ADC Hub Service"), SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
		cmdLine.c_str(), NULL, NULL, NULL, NULL, NULL);

	if (service == NULL)
	{
		PRINTERROR("CreateService");
		CloseServiceHandle(scm);
		return;
	}

	SERVICE_DESCRIPTION description = { const_cast<LPTSTR>(cmdLine.c_str()) };
	ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &description);

	fprintf(stdout, "Service \"%s\" successfully installed; command line:\n%s\n", serviceName, cmdLine.c_str());

	CloseServiceHandle(service);
	CloseServiceHandle(scm);
}

static void removeService()
{
	SC_HANDLE scm = OpenSCManager(NULL, NULL, STANDARD_RIGHTS_WRITE);
	if (scm == NULL)
	{
		PRINTERROR("OpenSCManager");
		return;
	}

	SC_HANDLE service = OpenService(scm, serviceName, DELETE);

	if (service == NULL)
	{
		PRINTERROR("OpenService");
		CloseServiceHandle(scm);
		return;
	}

	if (!DeleteService(service))
	{
		PRINTERROR("DeleteService");
		CloseServiceHandle(service);
		CloseServiceHandle(scm);
	}

	fprintf(stdout, "Service \"%s\" successfully removed\n", serviceName);

	CloseServiceHandle(service);
	CloseServiceHandle(scm);
}

static void init()
{
	// if(asService)
	// LOG(modName, versionString + " started as a service");
	// else
	// LOG(modName, versionString + " started from console");

	loadXML(*core, File::makeAbsolutePath(core->getConfigPath(), "config.xml"));
}

static void uninit()
{
	// LOG(modName, versionString + " shut down");
}

static SERVICE_STATUS_HANDLE ssh = 0;
static SERVICE_STATUS serviceStatus;

void WINAPI handler(DWORD code)
{
	switch (code)
	{
		case SERVICE_CONTROL_SHUTDOWN: // Fallthrough
		case SERVICE_CONTROL_STOP:
			if (core)
			{
				serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
				core->shutdown();
			}
			else
			{
				serviceStatus.dwCurrentState = SERVICE_STOPPED;
			}
			break;
		case SERVICE_CONTROL_INTERROGATE:
			break;
		default:; // LOG(modName, "Unknown service handler code " +
				  // Util::toString(code));
	}

	if (!SetServiceStatus(ssh, &serviceStatus))
	{
		// LOGERROR("handler::SetServiceStatus");
	}
}

static void WINAPI serviceStart(DWORD, TCHAR* argv[])
{
	ssh = ::RegisterServiceCtrlHandler(argv[0], handler);

	if (ssh == 0)
	{
		// LOGERROR("RegisterServiceCtrlHandler");
		uninit();
		return;
	}

	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 10 * 1000;

	if (!SetServiceStatus(ssh, &serviceStatus))
	{
		// LOGERROR("SetServiceStatus");
		uninit();
		return;
	}

	try
	{
		core = Core::create(configPath);

		init();
	}
	catch (const adchpp::Exception&)
	{
		// LOG(modName, "Failed to start: " + e.getError());
	}

	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ssh, &serviceStatus);

	try
	{
		core->run();
	}
	catch (const Exception&)
	{
		// LOG(modName, "ADCH++ startup failed because: " + e.getError());
	}

	serviceStatus.dwCurrentState = SERVICE_STOPPED;

	if (!SetServiceStatus(ssh, &serviceStatus))
	{
		// LOGERROR("SetServiceStatus");
		uninit();
		return;
	}

	uninit();
}

static void runService()
{
	SERVICE_TABLE_ENTRY DispatchTable[] = { { (LPTSTR)serviceName, &serviceStart }, { NULL, NULL } };

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		// LOGERROR("StartServiceCtrlDispatcher");
	}
}

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (core && dwCtrlType == CTRL_C_EVENT)
	{
		core->shutdown();
		return TRUE;
	}
	return FALSE;
}

static void runConsole()
{
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	printf("Starting %s\n", appName.c_str());
	try
	{
		core = Core::create(configPath);
		init();
		printf("%s running, press Ctrl-C to exit...\n", versionString.c_str());
		core->run();
		core.reset();
	}
	catch (const Exception& e)
	{
		fprintf(stderr, "\nFATAL: Can't start %s: %s\n", appName.c_str(), e.what());
	}

	uninit();
}

static void printUsage()
{
	const char* text =
		"Usage: " APPNAME " [[-c <configdir>] [-i <servicename> | -u <servicename>]] | [-v] | [-h]\n\n"
		"-c Specify the path of the configuration directory (default: .\\config)\n"
		"-i <service name> Install a service instance (name defaults to 'adchubd')\n"
		"-u <service name> Uninstall a service instance\n"
		"-v Print version number\n"
		"-h Show this help message\n";

	printf(text);
}

static void checkArg(int argc, char* argv[], int i)
{
	if (i + 1 == argc)
	{
		fprintf(stderr, "Parameter %s requires an argument\n", argv[i]);
		exit(1);
	}
}

int _tmain(int argc, TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	configPath = Util::getAppPath() + "config\\";
	int task = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (_tcscmp(argv[i], _T("-d")) == 0)
		{
			checkArg(argc, argv, i);
			serviceName = argv[++i];
			task = 1;
		}
		else if (_tcscmp(argv[i], _T("-c")) == 0)
		{
			checkArg(argc, argv, i);
			string cfg = argv[++i];
			if (cfg.empty())
			{
				printf("-c <directory>\n");
				return 2;
			}
			if (!File::isAbsolutePath(cfg))
			{
				printf("Config dir must be an absolute path\n");
				return 2;
			}
			if (cfg[cfg.length() - 1] != '\\') cfg += '\\';
			configPath = std::move(cfg);
		}
		else if (_tcscmp(argv[i], _T("-i")) == 0)
		{
			if (++i < argc) serviceName = argv[i];
			task = 2;
		}
		else if (_tcscmp(argv[i], _T("-u")) == 0)
		{
			if (++i < argc) serviceName = argv[i];
			task = 3;
		}
		else if (_tcscmp(argv[i], _T("-v")) == 0)
		{
			printf("%s compiled on " __DATE__ " " __TIME__ "\n", versionString.c_str());
			return 0;
		}
		else if (_tcscmp(argv[i], _T("-h")) == 0)
		{
			printUsage();
			return 0;
		}
		else
		{
			printf("Invalid parameter: %s\n", argv[i]);
			printUsage();
			return 4;
		}
	}

	switch (task)
	{
		case 0:
			runConsole();
			break;
		case 1:
			runService();
			break;
		case 2:
			installService();
			break;
		case 3:
			removeService();
			break;
	}

	return 0;
}
