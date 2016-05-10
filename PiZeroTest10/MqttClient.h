#include <mosquitto.h>

// Default MQTT port
#define MQTT_PORT 10891

// Default server name
#define MQTT_SERVERNAME "m21.cloudmqtt.com"

class MqttClient 
{
private:

	mosquitto * _data;
	const char * _userName;
	const char * _deviceId;
	const char * _devicePassword;
	const char * _serverName;
	bool _authenticatedInServer;
	bool connectIfNecessary();
	void initialize(const char * userName,const char * deviceId,const char * devicePassword,const char * serverName, void(*callback)(char*, char*, unsigned int));
  
public:

	MqttClient(const char * userName, const char * deviceId, const char * devicePassword); 
	MqttClient(char * userName, char * deviceId, char * devicePassword, char * serverName);
	MqttClient(char * userName, char * deviceId, char * devicePassword, void(*callback)(char*, char*, unsigned int));
	MqttClient(char * userName, char * deviceId, char * devicePassword, char * serverName, void(*callback)(char*, char*, unsigned int));
	~MqttClient();
	bool publish(const char * topic, const char * payload);
	bool subscribe(char * topic);
	bool loop();
	bool disconnect();

	  // We don't recommend use these functions directly.
	  // They are for internal purposes.
	void(*onMessage)(char*, char*, unsigned int);
	void(*onDisconnect)(void*);
	void resetConnectedState();
};