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

#include <adchpp/Core.h>
#include <adchpp/AppPaths.h>
#include <adchpp/version.h>
#include <baselib/File.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include "adchppd.h"

using namespace std;
using namespace adchpp;

static FILE* pidFile;
static string pidFileName;
static std::shared_ptr<Core> core;

static void installHandler();

void breakHandler(int)
{
	if (core) core->shutdown();
}

static void init()
{
	// Ignore SIGPIPE...
	struct sigaction sa = { 0 };

	sa.sa_handler = SIG_IGN;

	sigaction(SIGPIPE, &sa, nullptr);
	sigaction(SIGHUP, &sa, nullptr);

	sigset_t mask;

	sigfillset(&mask); // Mask all allowed signals, the other threads should inherit this
	sigdelset(&mask, SIGCONT);
	sigdelset(&mask, SIGFPE);
	sigdelset(&mask, SIGILL);
	sigdelset(&mask, SIGSEGV);
	sigdelset(&mask, SIGBUS);
	sigdelset(&mask, SIGINT);
	sigdelset(&mask, SIGTERM);
	sigdelset(&mask, SIGTRAP);
	pthread_sigmask(SIG_SETMASK, &mask, nullptr);

	installHandler();

	if (pidFile)
	{
		fprintf(pidFile, "%d", (int)getpid());
		fflush(pidFile);
	}

	loadXML(*core, AppPaths::makeAbsolutePath(core->getConfigPath(), "config.xml"));
}

static void installHandler()
{
	struct sigaction sa = { 0 };
	sa.sa_handler = breakHandler;
	sigaction(SIGINT, &sa, nullptr);
	sigaction(SIGTERM, &sa, nullptr);
}

static void uninit(bool asDaemon)
{
	if (!asDaemon) puts("Shut down");
	if (pidFile)
	{
		fclose(pidFile);
		pidFile = nullptr;
	}
	if (!pidFileName.empty()) unlink(pidFileName.c_str());
}

static void daemonize()
{
	switch (fork())
	{
		case -1:
			fprintf(stderr, "First fork failed: %s\n", strerror(errno));
			exit(5);
		case 0:
			break;
		default:
			exit(0);
	}

	if (setsid() < 0)
	{
		fprintf(stderr, "setsid failed: %s\n", strerror(errno));
		exit(6);
	}

	switch (fork())
	{
		case -1:
			fprintf(stderr, "Second fork failed: %s\n", strerror(errno));
			exit(7);
		case 0:
			break;
		default:
			exit(0);
	}

	chdir("/");
	close(0);
	close(1);
	close(2);
	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
}

static void run(const string& configPath, bool asDaemon)
{
	if (asDaemon)
		daemonize();
	else
		printf("Starting %s\n", appName.c_str());
	try
	{
		core = Core::create(configPath);
		init();
		if (!asDaemon) printf("%s running, press Ctrl-C to exit...\n", versionString.c_str());
		core->run();
		core.reset();
	}
	catch (const Exception& e)
	{
		fprintf(stderr, "\nFATAL: Can't start %s: %s\n", appName.c_str(), e.what());
	}
	uninit(asDaemon);
}

static void printUsage()
{
	printf("Usage: " APPNAME " [[-c <configdir>] [-d]] | [-v] | [-h]\n");
}

static void checkArg(int argc, char* argv[], int i)
{
	if (i + 1 == argc)
	{
		fprintf(stderr, "Parameter %s requires an argument\n", argv[i]);
		exit(1);
	}
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

	string configPath = "/etc/" APPNAME "/";
	bool asDaemon = false;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-d") == 0)
		{
			asDaemon = true;
		}
		else if (strcmp(argv[i], "-v") == 0)
		{
			printf("%s\n", versionString.c_str());
			return 0;
		}
		else if (strcmp(argv[i], "-c") == 0)
		{
			checkArg(argc, argv, i);
			string cfg = argv[++i];
			if (cfg.empty() || cfg[0] != '/')
			{
				fprintf(stderr, "Config dir must be an absolute path\n");
				return 2;
			}

			if (cfg[cfg.length() - 1] != '/') cfg += '/';
			configPath = std::move(cfg);
		}
		else if (strcmp(argv[i], "-p") == 0)
		{
			checkArg(argc, argv, i);
			pidFileName = argv[++i];
		}
		else if (strcmp(argv[i], "-h") == 0)
		{
			printUsage();
			return 0;
		}
		else
		{
			fprintf(stderr, "Unknown parameter: %s\n", argv[i]);
			return 4;
		}
	}

	if (!pidFileName.empty())
	{
		pidFileName = AppPaths::makeAbsolutePath("/run/", pidFileName);
		pidFile = fopen(pidFileName.c_str(), "w");
		if (!pidFile)
		{
			fprintf(stderr, "Can't open %s for writing\n", pidFileName.c_str());
			return 1;
		}
	}

	run(configPath, asDaemon);
	return 0;
}
