#include "stdafx.h"

#include "IpUpdatesLog.h"
#include "MiscUtil.h"
#include "StrUtil.h"

/*
TODO:
* finish writing out the truncated log
*/

// When we reach that many updates, we'll trim the ip updates log
#define IP_UPDATES_PURGE_LIMIT 1000

// After we reach IP_UPDATES_PURGE_LIMIT, we'll only write out
// IP_UPDATES_AFTER_PURGE_SIZE items, so that we don't have to purge every
// time once we reach purge limit
#define IP_UPDATES_SIZE_AFTER_PURGE 500

// Linked list of ip updates. Newest are at the front.
IpUpdate *				g_ipUpdates = NULL;

// name of the file where we persist ip updates history
static const TCHAR *	gIpUpdatesLogFileName;
static FILE *			gIpUpdatesLogFile;

static void FreeIpUpdate(IpUpdate *ipUpdate)
{
	assert(ipUpdate);
	if (!ipUpdate)
		return;
	free(ipUpdate->ipAddress);
	free(ipUpdate->time);
	free(ipUpdate);
}

static void FreeIpUpdatesFromElement(IpUpdate *curr)
{
	while (curr) {
		IpUpdate *next = curr->next;
		FreeIpUpdate(curr);
		curr = next;
	}
}

static void InsertIpUpdate(const char *ipAddress, const char *time)
{
	assert(ipAddress);
	assert(time);
	if (!ipAddress || !time)
		return;
	IpUpdate *ipUpdate = SA(IpUpdate);
	if (!ipUpdate)
		return;

	ipUpdate->ipAddress = strdup(ipAddress);
	ipUpdate->time = strdup(time);
	if (!ipUpdate->ipAddress || !ipUpdate->time) {
		FreeIpUpdate(ipUpdate);
		return;
	}

	ipUpdate->next = g_ipUpdates;
	g_ipUpdates = ipUpdate;
}

static inline bool is_newline_char(char c)
{
	return (c == '\r') || (c == '\n');
}

// at this point <*dataStartInOut> points at the beginning of the log file,
// which consists of lines in format:
// $ipaddr $time\r\n
static bool ExtractIpAddrAndTime(char **dataStartInOut, uint64_t *dataSizeLeftInOut, char **ipAddrOut, char **timeOut)
{
	char *curr = *dataStartInOut;
	uint64_t dataSizeLeft = *dataSizeLeftInOut;
	char *time = NULL;
	char *ipAddr = curr;

	// first space separates $ipaddr from $time
	while ((dataSizeLeft > 0) && (*curr != ' ')) {
		--dataSizeLeft;
		++curr;
	}

	// didn't find the space => something's wrong
	if (0 == dataSizeLeft)
		return false;

	assert(*curr == ' ');
	// replace space with 0 to make ipAddr a null-terminated string
	*curr = 0;
	--dataSizeLeft;
	++curr;

	time = curr;

	// find "\r\n' at the end
	while ((dataSizeLeft > 0) && !is_newline_char(*curr)) {
		--dataSizeLeft;
		++curr;
	}

	// replace '\r\n' with 0, to make time a null-terminated string
	while ((dataSizeLeft > 0) && is_newline_char(*curr)) {
		*curr++ = 0;
		--dataSizeLeft;
	}

	*ipAddrOut = ipAddr;
	*timeOut = time;
	*dataSizeLeftInOut = dataSizeLeft;
	*dataStartInOut = curr;
	return true;
}

// load up to IP_UPDATES_HISTORY_MAX latest entries from ip updates log
// When we finish g_ipUpdates is a list of ip updates with the most recent
// at the beginning of the list
static void ParseIpLogHistory(char *data, uint64_t dataSize)
{
	char *ipAddr = NULL;
	char *time = NULL;
	while (dataSize != 0) {
		bool ok = ExtractIpAddrAndTime(&data, &dataSize, &ipAddr, &time);
		if (!ok) {
			assert(0);
			break;
		}
		InsertIpUpdate(ipAddr, time);
	}
}

static void LogIpUpdateEntry(FILE *log, const char *ipAddress, const char *time)
{
	assert(log && ipAddress && time);
	if (!log || !ipAddress || !time)
		return;

	size_t slen = strlen(ipAddress);
	fwrite(ipAddress, slen, 1, log);
	fwrite(" ", 1, 1, log);

	slen = strlen(time);
	fwrite(time, slen, 1, log);
	fwrite("\r\n", 2, 1, log);
	fflush(log);
}

#if 0
// overwrite the history log file with the current history in g_ipUpdates
// The assumption is that we've limited 
static void WriteIpLogHistory(const TCHAR *logFileName)
{
	FILE *log = _tfopen(logFileName, _T("wb"));

	fclose(log);
}

static void RemoveLogEntries(int max)
{
	IpUpdate *curr = g_ipUpdates;
	IpUpdate **currPtr = NULL;
	while (curr && max > 0) {
		currPtr = &curr->next;
		curr = curr->next;
		--max;
	}
	if (!curr || !currPtr)
		return;
	FreeIpUpdatesFromElement(*currPtr);
	*currPtr = NULL;
}
#endif

static void LoadAndParseHistory(const TCHAR *logFileName)
{
	uint64_t dataSize;
	char *data = FileReadAll(logFileName, &dataSize);
	if (!data)
		return;
	ParseIpLogHistory(data, dataSize);
	free(data);
}

void LoadIpUpdatesHistory(const TCHAR *logFileName)
{
	assert(!gIpUpdatesLogFileName);
	assert(!gIpUpdatesLogFile);

	LoadAndParseHistory(logFileName);

	gIpUpdatesLogFileName = tstrdup(logFileName);
	gIpUpdatesLogFile = _tfopen(logFileName, _T("ab"));
}

void LogIpUpdate(const char *ipAddress)
{
	char timeBuf[256];
	__time64_t ltime;
	struct tm *today;
	today = _localtime64(&ltime);
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", today);
	assert(gIpUpdatesLogFile);
	LogIpUpdateEntry(gIpUpdatesLogFile, ipAddress, timeBuf);
}

static void CloseIpUpdatesLog()
{
	TStrFree(&gIpUpdatesLogFileName);
	if (gIpUpdatesLogFile) {
		fclose(gIpUpdatesLogFile);
		gIpUpdatesLogFile = NULL;
	}
}

void FreeIpUpdatesHistory()
{
	CloseIpUpdatesLog();
	// TODO: if we have more than IP_UPDATES_HISTORY_MAX entries, over-write
	// the log with only the recent entries
	FreeIpUpdatesFromElement(g_ipUpdates);
	g_ipUpdates = NULL;
}
