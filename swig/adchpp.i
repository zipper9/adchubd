%runtime %{

#include <adchpp/Signal.h>
#include <adchpp/Client.h>
#include <adchpp/ClientManager.h>
#include <adchpp/LogManager.h>
#include <adchpp/PluginManager.h>
#include <adchpp/SocketManager.h>
#include <adchpp/Hub.h>
#include <adchpp/Bot.h>
#include <adchpp/Core.h>
#include <adchpp/Utils.h>
#include <adchpp/version.h>
#include <baselib/TigerHash.h>
#include <baselib/Text.h>
#include <baselib/SimpleXML.h>
#include <baselib/FormatUtil.h>

using namespace adchpp;
using std::shared_ptr;
using std::make_shared;

%}

%include "exception.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_except.i"
%include "std_pair.i"
%include "std_map.i"

%include "carrays.i"

%array_functions(size_t, size_t);

%exception {
	try {
		$action
	} catch(const std::exception& e) {
		SWIG_exception(SWIG_UnknownError, e.what());
	}
}

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned int time_t;

using namespace std;

template<typename T>
struct shared_ptr {
	T* operator->();
};

%nodefaultctor;
%nodefaultdtor Entity;
%nodefaultdtor Client;
%nodefaultdtor ClientManager;
%nodefaultdtor SocketManager;
%nodefaultdtor LogManager;
%nodefaultdtor Text;
%nodefaultdtor Util;
%nodefaultdtor PluginManager;

namespace adchpp {
	class Bot;
	class Client;
	class Entity;
}

typedef std::vector<std::string> StringList;
typedef std::vector<uint8_t> ByteVector;

%template(TServerInfoPtr) shared_ptr<adchpp::ServerInfo>;
%template(TManagedConnectionPtr) shared_ptr<adchpp::ManagedConnection>;
%template(TEntityList) std::vector<adchpp::Entity*>;
%template(TStringList) std::vector<std::string>;
%template(TByteVector) std::vector<uint8_t>;
%template(TServerInfoList) std::vector<shared_ptr<adchpp::ServerInfo> >;

%inline%{
	namespace adchpp {
	typedef std::vector<adchpp::Entity*> EntityList;
	}
%}

/* some SWIG hackery: SWIG can't parse nested C++ structs so we take TLSInfo out as a global class,
then redeclare it with a typedef */
struct TLSInfo {
	std::string cert;
	std::string pkey;
	std::string trustedPath;
	std::string dh;
};
%{
typedef ServerInfo::TLSInfo TLSInfo;
%}

class Exception : public std::exception
{
public:
	Exception();
	Exception(const std::string& aError) throw();
	virtual ~Exception() throw();
	const std::string& getError() const throw();

	virtual const char* what();
};

class SimpleXML
{
public:
	SimpleXML();
	~SimpleXML();

	void addTag(const std::string& name, const std::string& data = Util::emptyString);
	void addAttrib(const std::string& name, const std::string& data);
	void addChildAttrib(const std::string& name, const std::string& data);

	const std::string& getData() const;
	void stepIn();
	void stepOut();

	void resetCurrentChild();
	bool findChild(const std::string& name) noexcept;

	const std::string& getChildTag() const;
	const std::string& getChildData() const;

	const std::string& getChildAttrib(const std::string& name, const std::string& defValue = Util::emptyString);

	int getIntChildAttrib(const std::string& name, const string& defValue);
	int64_t getInt64ChildAttrib(const std::string& name, const string& defValue);
	bool getBoolChildAttrib(const std::string& name);
	void fromXML(const std::string& xml);
	std::string toXML() const;

	static const std::string& escape(const std::string& str, std::string& tmp, bool isAttrib, bool isLoading = false, int encoding = Text::CHARSET_UTF8);
	static bool needsEscape(const std::string& str, bool isAttrib, bool isLoading, int encoding = Text::CHARSET_UTF8);
};

class TigerHash
{
public:
	/** Hash size in bytes */
	enum { BITS = 192, BYTES = BITS / 8 }; // Keep old name for a while

	TigerHash();

	%extend {
		void update(const std::string& data) {
			self->update(data.data(), data.size());
		}
		std::string finalize() {
			return std::string(reinterpret_cast<const char*>(self->finalize()), TigerHash::BYTES);
		}
	}
};

namespace adchpp
{

extern std::string appName;
extern std::string versionString;
extern float versionFloat;

enum Reason
{
	REASON_BAD_STATE,
	REASON_CID_CHANGE,
	REASON_CID_TAKEN,
	REASON_FLOODING,
	REASON_HUB_FULL,
	REASON_INVALID_COMMAND_TYPE,
	REASON_INVALID_IP,
	REASON_INVALID_SID,
	REASON_LOGIN_TIMEOUT,
	REASON_MAX_COMMAND_SIZE,
	REASON_NICK_INVALID,
	REASON_NICK_TAKEN,
	REASON_NO_BASE_SUPPORT,
	REASON_NO_TIGR_SUPPORT,
	REASON_PID_MISSING,
	REASON_PID_CID_LENGTH,
	REASON_PID_CID_MISMATCH,
	REASON_PID_WITHOUT_CID,
	REASON_PLUGIN,
	REASON_WRITE_OVERFLOW,
	REASON_NO_BANDWIDTH,
	REASON_INVALID_DESCRIPTION,
	REASON_WRITE_TIMEOUT,
	REASON_SOCKET_ERROR,
	REASON_LAST
};

class Buffer
{
public:
	%extend {
		static shared_ptr<Buffer> create(const std::string& s) {
			return make_shared<Buffer>(s);
		}
	}
};

typedef shared_ptr<Buffer> BufferPtr;

struct ManagedConnection {
	void disconnect();
	void release();
};

typedef shared_ptr<ManagedConnection> ManagedConnectionPtr;

struct ServerInfo {
	std::string bind4;
	std::string bind6;
	std::string address4;
	std::string address6;
	std::string port;

	TLSInfo TLSParams;
	bool secure() const;

	%extend {
		static shared_ptr<adchpp::ServerInfo> create() {
			return make_shared<ServerInfo>();
		}
	}
};

typedef shared_ptr<ServerInfo> ServerInfoPtr;
typedef std::vector<ServerInfoPtr> ServerInfoList;

struct SocketStats {
	SocketStats() : queueCalls(0), queueBytes(0), sendCalls(0), sendBytes(0), recvCalls(0), recvBytes(0) { }

	size_t queueCalls;
	int64_t queueBytes;
	size_t sendCalls;
	int64_t sendBytes;
	int64_t recvCalls;
	int64_t recvBytes;
};

class SocketManager {
public:
	typedef std::function<void()> Callback;
	%extend {
		/* work around 2 problems:
		- SWIG fails to convert a script function to const Callback&.
		- SWIG has trouble choosing the overload of addJob / addTimedJob to use.
		*/
		void addJob(const long msec, Callback callback) {
			self->addJob(msec, callback);
		}
		void addJob_str(const std::string& time, Callback callback) {
			self->addJob(time, callback);
		}
		Callback addTimedJob(const long msec, Callback callback) {
			return self->addTimedJob(msec, callback);
		}
		Callback addTimedJob_str(const std::string& time, Callback callback) {
			return self->addTimedJob(time, callback);
		}
	}

	void setBufferSize(size_t newSize);
	size_t getBufferSize() const;

	void setMaxBufferSize(size_t newSize);
	size_t getMaxBufferSize() const;

	void setOverflowTimeout(size_t timeout);
	size_t getOverflowTimeout() const;

    void setDisconnectTimeout(size_t timeout);
	size_t getDisconnectTimeout() const;

	void setServers(const ServerInfoList& servers_);

	SocketStats &getStats();
};

template<typename F>
struct Signal {
%extend {
	ManagedConnectionPtr connect(std::function<F> f) {
		return manage(self, f);
	}
}
};

template<typename F>
struct SignalTraits {
	typedef adchpp::Signal<F> Signal;
	//typedef adchpp::ConnectionPtr Connection;
	typedef adchpp::ManagedConnectionPtr ManagedConnection;
};

// SWIG doesn't like nested classes
%{
typedef adchpp::Reason DCReason;
%}

class CID
{
public:
	enum { SIZE = 192 / 8 };
	enum { BASE32_SIZE = 39 };

	CID();
	explicit CID(const uint8_t* data);
	explicit CID(const std::string& base32);

	bool operator==(const CID& rhs) const;
	bool operator<(const CID& rhs) const;

	std::string toBase32() const;
	//std::string& toBase32(std::string& tmp) const;

	size_t toHash() const;
	//const uint8_t* data() const;

	%extend {
		std::string data() const { return std::string(reinterpret_cast<const char*>($self->data()), CID::SIZE); }
		std::string __str__() { return $self->toBase32(); }
	}

	bool isZero() const;
	static CID generate();

};

class AdcCommand
{
public:
	enum Error
	{
		ERROR_GENERIC = 0,
		ERROR_HUB_GENERIC = 10,
		ERROR_HUB_FULL = 11,
		ERROR_HUB_DISABLED = 12,
		ERROR_LOGIN_GENERIC = 20,
		ERROR_NICK_INVALID = 21,
		ERROR_NICK_TAKEN = 22,
		ERROR_BAD_PASSWORD = 23,
		ERROR_CID_TAKEN = 24,
		ERROR_COMMAND_ACCESS = 25,
		ERROR_REGGED_ONLY = 26,
		ERROR_INVALID_PID = 27,
		ERROR_BANNED_GENERIC = 30,
		ERROR_PERM_BANNED = 31,
		ERROR_TEMP_BANNED = 32,
		ERROR_PROTOCOL_GENERIC = 40,
		ERROR_PROTOCOL_UNSUPPORTED = 41,
		ERROR_INF_MISSING = 42,
		ERROR_BAD_STATE = 43,
		ERROR_FEATURE_MISSING = 44,
		ERROR_BAD_IP = 45,
		ERROR_TRANSFER_GENERIC = 50,
		ERROR_FILE_NOT_AVAILABLE = 51,
		ERROR_FILE_PART_NOT_AVAILABLE = 52,
		ERROR_SLOTS_FULL = 53
	};

	enum Severity
	{
		SEV_SUCCESS = 0,
		SEV_RECOVERABLE = 1,
		SEV_FATAL = 2
	};

	enum Priority
	{
		PRIORITY_NORMAL,
		PRIORITY_LOW,
		PRIORITY_IGNORE
	};

	static const char TYPE_BROADCAST = 'B';
	static const char TYPE_CLIENT = 'C';
	static const char TYPE_DIRECT = 'D';
	static const char TYPE_ECHO = 'E';
	static const char TYPE_FEATURE = 'F';
	static const char TYPE_INFO = 'I';
	static const char TYPE_HUB = 'H';
	static const char TYPE_UDP = 'U';

	// Known commands...
#define C(n, a, b, c) static const unsigned int CMD_##n = (((uint32_t)a) | (((uint32_t)b)<<8) | (((uint32_t)c)<<16));
	// Base commands
	C(SUP, 'S','U','P');
	C(STA, 'S','T','A');
	C(INF, 'I','N','F');
	C(MSG, 'M','S','G');
	C(SCH, 'S','C','H');
	C(RES, 'R','E','S');
	C(CTM, 'C','T','M');
	C(RCM, 'R','C','M');
	C(GPA, 'G','P','A');
	C(PAS, 'P','A','S');
	C(QUI, 'Q','U','I');
	C(GET, 'G','E','T');
	C(GFI, 'G','F','I');
	C(SND, 'S','N','D');
	C(SID, 'S','I','D');
	// Extensions
	C(CMD, 'C','M','D');
	C(NAT, 'N','A','T');
	C(RNT, 'R','N','T');
#undef C

	static const uint32_t HUB_SID;
	static const uint32_t INVALID_SID;
	
	AdcCommand();
	
	explicit AdcCommand(Severity sev, Error err, const std::string& desc, char aType);
	explicit AdcCommand(uint32_t cmd, char aType, uint32_t aFrom);
	explicit AdcCommand(const std::string& aLine) throw(ParseException);
	explicit AdcCommand(const BufferPtr& buffer_) throw(ParseException);
	
	static uint32_t toSID(const std::string& aSID);
	static std::string fromSID(const uint32_t aSID);
	static void appendSID(std::string& str, uint32_t aSID);

	static uint32_t toCMD(uint8_t a, uint8_t b, uint8_t c);
	//static uint32_t toCMD(const char* str);

	static uint16_t toField(const char* x);
	static std::string fromField(const uint16_t aField);

	static uint32_t toFourCC(const char* x);
	static std::string fromFourCC(uint32_t x);

	void parse(const std::string& aLine) throw(ParseException);
	uint32_t getCommand() const;
	char getType() const;
	std::string getFourCC() const;
	StringList& getParameters();
	//const StringList& getParameters() const;

	std::string toString() const;
	void resetBuffer();

	AdcCommand& addParam(const std::string& name, const std::string& value);
	AdcCommand& addParam(const std::string& str);
	const std::string& getParam(size_t n) const;

	const std::string& getFeatures() const;

	const BufferPtr& getBuffer() const;

#ifndef SWIGLUA
	bool getParam(const char* name, size_t start, std::string& OUTPUT) const;
#endif
	bool delParam(const char* name, size_t start);

	bool hasFlag(const char* name, size_t start) const;

	bool operator==(uint32_t aCmd) const;

	static void escape(const std::string& s, std::string& out);

	uint32_t getTo() const;
	void setTo(uint32_t aTo);
	uint32_t getFrom() const;
	void setFrom(uint32_t aFrom);
	
	Priority getPriority() const { return priority; }
	void setPriority(Priority priority_) { priority = priority_; }

%extend {
	std::string getCommandString() {
		int cmd = self->getCommand();
		return std::string(reinterpret_cast<const char*>(&cmd), 3);
	}
	static uint32_t toCMD(const std::string& cmd) {
		if(cmd.length() != 3) {
			return 0;
		}
		return (((uint32_t)cmd[0]) | (((uint32_t)cmd[1])<<8) | (((uint32_t)cmd[2])<<16));
	}
}

};

class Entity {
public:
	enum State {
		/** Initial protocol negotiation (wait for SUP) */
		STATE_PROTOCOL,
		/** Identify the connecting client (wait for INF) */
		STATE_IDENTIFY,
		/** Verify the client (wait for PAS) */
		STATE_VERIFY,
		/** Normal operation */
		STATE_NORMAL,
		/** Binary data transfer */
		STATE_DATA
	};

	enum Flag {
		FLAG_BOT = 0x01,
		FLAG_REGISTERED = 0x02,
		FLAG_OP = 0x04,
		FLAG_SU = 0x08,
		FLAG_OWNER = 0x10,
		FLAG_HUB = 0x20,
		FLAG_HIDDEN = 0x40,
		MASK_CLIENT_TYPE = FLAG_BOT | FLAG_REGISTERED | FLAG_OP | FLAG_SU | FLAG_OWNER | FLAG_HUB | FLAG_HIDDEN,

		FLAG_PASSWORD = 0x100,

		/** Extended away, no need to send msg */
		FLAG_EXT_AWAY = 0x200,

		/** Plugins can use these flags to disable various checks */
		/** Bypass ip check */
		FLAG_OK_IP = 0x400,

		/** This entity is now a ghost being disconnected, totally ignored by ADCH++ */
		FLAG_GHOST = 0x800
	};

	Entity(uint32_t sid_) : sid(sid_) {}

	void send(const AdcCommand& cmd) { send(cmd.getBuffer()); }
	virtual void send(const BufferPtr& cmd) = 0;

	virtual void inject(AdcCommand& cmd);

	const std::string& getField(const char* name) const;
	bool hasField(const char* name) const;
	void setField(const char* name, const std::string& value);

	/** Add any flags that have been updated to the AdcCommand (type etc is not set) */
	bool getAllFields(AdcCommand& cmd) const throw();
	const BufferPtr& getINF() const;

	bool addSupports(uint32_t feature);
	StringList getSupportList() const;
	bool hasSupport(uint32_t feature) const;
	bool removeSupports(uint32_t feature);

	const BufferPtr& getSUP() const;

	uint32_t getSID() const;

	bool isFiltered(const std::string& features) const;

	void updateFields(const AdcCommand& cmd);
	void updateSupports(const AdcCommand& cmd) throw();

	const CID& getCID() const { return cid; }
	void setCID(const CID& cid_) { cid = cid_; }

	State getState() const { return state; }
	void setState(State state_) { state = state_; }

	bool isSet(size_t aFlag) const { return flags.isSet(aFlag); }
	bool isAnySet(size_t aFlag) const { return flags.isAnySet(aFlag); }
	void setFlag(size_t aFlag);
	void unsetFlag(size_t aFlag);

%extend {
	Client* asClient()
	{
		return $self->getType() == Entity::TYPE_CLIENT ? static_cast<Client*>($self) : nullptr;
	}
	Hub* asHub()
	{
		return $self->getType() == Entity::TYPE_HUB ? static_cast<Hub*>($self) : nullptr;
	}
	Bot* asBot()
	{
		return $self->getType() == Entity::TYPE_BOT ? static_cast<Bot*>($self) : nullptr;
	}
}

};

/**
 * The client represents one connection to a user.
 */
class Client : public Entity {
public:
	// static Client* create(const ManagedSocketPtr& ms_, uint32_t sid_) throw();

	using Entity::send;

	virtual void send(const BufferPtr& command);

	size_t getQueuedBytes() throw();

	const std::string& getIp() const throw();

	/**
	 * Set data mode for aBytes bytes.
	 * May only be called from on(ClientListener::Command...).
	 */
	typedef std::function<void (Client&, const uint8_t*, size_t)> DataFunction;
	void setDataMode(const DataFunction& handler, int64_t aBytes);

	%extend{
		void disconnect(int reason) {
			self->disconnect(static_cast<DCReason>(reason), Util::emptyString);
		}
		void disconnect(int reason, const std::string& info) {
			self->disconnect(static_cast<DCReason>(reason), info);
		}
	}
};

class Bot : public Entity {
public:
	typedef std::function<void (Bot& bot, const BufferPtr& cmd)> SendHandler;

	using Entity::send;
	virtual void send(const BufferPtr& cmd);

	%extend{
		void disconnect(int reason) {
			self->disconnect(static_cast<DCReason>(reason), Util::emptyString);
		}
		void disconnect(int reason, const std::string& info) {
			self->disconnect(static_cast<DCReason>(reason), info);
		}
	}
};

class Hub : public Entity {
public:
	virtual void send(const BufferPtr& cmd);

private:
};

%template(SignalE) Signal<void (Entity&)>;
%template(SignalTraitsE) SignalTraits<void (Entity&)>;

%template(SignalEAB) Signal<void (Entity&, AdcCommand&, bool&)>;
%template(SignalTraitsEAB) SignalTraits<void (Entity&, AdcCommand&, bool&)>;

%template(SignalES) Signal<void (Entity&, const std::string&)>;
%template(SignalTraitsES) SignalTraits<void (Entity&, const std::string&)>;

%template(SignalEcAB) Signal<void (Entity&, const AdcCommand&, bool&)>;
%template(SignalTraitsEcAB) SignalTraits<void (Entity&, const AdcCommand&, bool&)>;

%template(SignalEI) Signal<void (Entity&, int)>;
%template(SignalTraitsEI) SignalTraits<void (Entity&, int)>;

%template(SignalERS) Signal<void (Entity&, DCReason, const std::string &)>;
%template(SignalTraitsERS) SignalTraits<void (Entity&, DCReason, const std::string &)>;

%template(SignalESB) Signal<void (Entity&, const StringList&, bool&)>;
%template(SignalTraitsESB) SignalTraits<void (Entity&, const StringList&, bool&)>;

%template(SignalS) Signal<void (const SimpleXML&)>;
%template(SignalTraitsS) SignalTraits<void (const SimpleXML&)>;

%template(SignalSt) Signal<void (const std::string&)>;
%template(SignalTraitsSt) SignalTraits<void (const std::string&)>;

class LogManager
{
public:
	void log(const std::string& area, const std::string& msg) throw();

	void setLogFile(const std::string& fileName);
	const std::string& getLogFile() const;

	void setEnabled(bool enabled_);
	bool getEnabled() const;
	typedef SignalTraits<void (const std::string&)> SignalLog;
	SignalLog::Signal& signalLog() { return signalLog_; }
};

class ClientManager
{
public:
	Entity* getEntity(uint32_t aSid) throw();

	%extend{
	Bot* createBot(Bot::SendHandler handler) {
		return self->createBot(handler);
	}
	Bot* createSimpleBot() {
		return self->createBot(Bot::SendHandler());
	}
	}
	void regBot(Bot& bot);

	%extend{

	EntityList getEntities() throw() {
		EntityList ret;
		for(ClientManager::EntityMap::iterator i = self->getEntities().begin(); i != self->getEntities().end(); ++i) {
			ret.push_back(i->second);
		}
		return ret;
	}

	Entity* findByCID(const CID& cid) {
		return self->getEntity(self->getSID(cid));
	}

	Entity* findByNick(const std::string& nick) {
		return self->getEntity(self->getSID(nick));
	}

	}

	void send(const AdcCommand& cmd) throw();
	void sendToAll(const BufferPtr& buffer) throw();
	void sendTo(const BufferPtr& buffer, uint32_t to);

	void enterIdentify(Entity& c, bool sendData) throw();

	ByteVector enterVerify(Entity& c, bool sendData) throw();
	bool enterNormal(Entity& c, bool sendData, bool sendOwnInf) throw();
	bool verifySUP(Entity& c, AdcCommand& cmd) throw();
	bool verifyINF(Entity& c, AdcCommand& cmd) throw();
	bool verifyPassword(Entity& c, const std::string& password, const ByteVector& salt, const std::string& suppliedHash);
	bool verifyOverflow(Entity& c);
	void setState(Entity& c, Entity::State newState) throw();
	size_t getQueuedBytes() throw();

	typedef SignalTraits<void (Entity&)> SignalConnected;
	typedef SignalTraits<void (Entity&)> SignalReady;
	typedef SignalTraits<void (Entity&, AdcCommand&, bool&)> SignalReceive;
	typedef SignalTraits<void (Entity&, const std::string&)> SignalBadLine;
	typedef SignalTraits<void (Entity&, const AdcCommand&, bool&)> SignalSend;
	typedef SignalTraits<void (Entity&, int)> SignalState;
	typedef SignalTraits<void (Entity&, DCReason, const std::string &)> SignalDisconnected;

	SignalConnected::Signal& signalConnected() { return signalConnected_; }
	SignalReady::Signal& signalReady() { return signalReady_; }
	SignalReceive::Signal& signalReceive() { return signalReceive_; }
	SignalBadLine::Signal& signalBadLine() { return signalBadLine_; }
	SignalSend::Signal& signalSend() { return signalSend_; }
	SignalState::Signal& signalState() { return signalState_; }
	SignalDisconnected::Signal& signalDisconnected() { return signalDisconnected_; }
	
	void setMaxCommandSize(size_t newSize);
	size_t getMaxCommandSize() const;

	void setLogTimeout(size_t millis);
	size_t getLogTimeout() const;

	//virtual ~ClientManager() throw() { }
};

class Plugin {
public:
	virtual ~Plugin();
	/** @return API version for a plugin (incremented every time API changes) */
	virtual int getVersion() = 0;

protected:
	Plugin();
};

class PluginManager
{
public:
	%extend {
		void attention(std::function<void()> f) {
			self->attention(f);
		}
	}

	//typedef HASH_MAP<std::string, Plugin*> Registry;
	//typedef Registry::iterator RegistryIter;

 	const StringList& getPluginList() const;
 	const std::string& getPluginPath() const;
	void setPluginList(const StringList& pluginList);
	void setPluginPath(const std::string& name);
	
	//int getPluginId() { return pluginIds++; }

	bool registerPlugin(const std::string& name, shared_ptr<Plugin> ptr);
	bool unregisterPlugin(const std::string& name);
	shared_ptr<Plugin> getPlugin(const std::string& name);
	//const Registry& getPlugins();
	//void load();
	//void shutdown();

	typedef SignalTraits<void (Entity&, const StringList&, bool&)>::Signal CommandSignal;
	typedef CommandSignal::Slot CommandSlot;
	%extend {
		ManagedConnectionPtr onCommand(const std::string& commandName, CommandSlot f) {
			return ManagedConnectionPtr(new ManagedConnection(self->onCommand(commandName, f)));
		}
	}
	CommandSignal& getCommandSignal(const std::string& commandName);
};

}
