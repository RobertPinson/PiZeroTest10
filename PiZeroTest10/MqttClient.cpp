#include <iostream>
#include <cstring>
#include "MqttClient.h"

using namespace std;

/*
 * Internal callback for processing incoming messages. 
 * It calls more friendly callback supplied by user.
 */

static void callbackMessageReceived(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	MqttClient * _this = (MqttClient *)mosq;

	if (_this->onMessage != 0)
	{
		mosquitto_message * destination = new mosquitto_message();
		mosquitto_message_copy(destination, message);   
		_this->onMessage(destination->topic, (char*)destination->payload, destination->payloadlen);
	}
}


/*
 * Internal callback for processing disconnect.
 * We reset authenticated flag to force restore server connection next time.
 */
static void callbackDisconnected(struct mosquitto *mosq, void *userdata, int result)
{
	MqttClient * _this = (MqttClient *)mosq;
	_this->resetConnectedState();
}

MqttClient::MqttClient(const char * userName, 
	const char * deviceId, 
	const char * devicePassword)
{
	this->initialize(userName, deviceId, devicePassword, MQTT_SERVERNAME, 0);
}

MqttClient::MqttClient(char * userName, 
	char * deviceId, 
	char * devicePassword, 
	char * serverName)
{
	this->initialize(userName, deviceId, devicePassword, serverName, 0);  
}

MqttClient::MqttClient(char * userName, 
	char * deviceId, 
	char * devicePassword, 
	void(*callback)(char*, char*, unsigned int))
{
	this->initialize(userName, deviceId, devicePassword, MQTT_SERVERNAME, callback);  
}


MqttClient::MqttClient(char * userName, 
	char * deviceId, 
	char * devicePassword, 
	char * serverName, 
	void(*onMessage)(char*, char*, unsigned int))
{
	this->initialize(userName, deviceId, devicePassword, serverName, onMessage);  
}


/*
 * Startup initializer (called from constructors).
 * Do not use it directly.
 */
void MqttClient::initialize(const char * userName, 
	const char * deviceId, 
	const char * devicePassword, 
	const char * serverName, 
	void(*onMessage)(char*, char*, unsigned int))
{
	mosquitto_lib_init();

	this->_data = mosquitto_new(deviceId, false, this);

	this->_userName = userName;
	this->_devicePassword = devicePassword;
	this->_serverName = serverName;
	this->_authenticatedInServer = false;
	this->onMessage = onMessage;
		
	mosquitto_message_callback_set(this->_data, callbackMessageReceived);
	mosquitto_disconnect_callback_set(this->_data, callbackDisconnected);
}


/*
 * Free allocated resources.
 */
MqttClient::~MqttClient()
{
	mosquitto_disconnect(this->_data);
	mosquitto_destroy(this->_data);
	mosquitto_lib_cleanup();
}


/*
 * Perform connection to server (if not already connected) and return TRUE if success.
 */
bool MqttClient::connectIfNecessary()
{
	if (this->_authenticatedInServer)
	{
		return true;
	}

	  // Call it before mosquitto_connect to supply additional user credentials.
	mosquitto_username_pw_set(this->_data, this->_userName, this->_devicePassword);

	int result = mosquitto_connect(this->_data, this->_serverName, MQTT_PORT, 60);
	this->_authenticatedInServer = result == MOSQ_ERR_SUCCESS;
	return this->_authenticatedInServer;
}


/*
 * Reset connection flag to "not connected".
 */
void MqttClient::resetConnectedState()
{
	this->_authenticatedInServer = false;
}


/*
 * Publish message to topic.
 */
bool MqttClient::publish(const char* topic, const char* payload) 
{
	if (!this->connectIfNecessary())
	{
		return false;
	}

	int result = mosquitto_publish(this->_data, 0, topic, strlen(payload), payload, 1, false);

	return result == MOSQ_ERR_SUCCESS;
}


/*
 * Subscribe for topic.
 */
bool MqttClient::subscribe(char* topic)
{
	if (!this->connectIfNecessary())
	{
		return false;
	}

	int result = mosquitto_subscribe(this->_data, 0, topic, 0);

	return result == MOSQ_ERR_SUCCESS;
}


/*
 * The main network loop for the client. You must call this frequently in order
 * to keep communications between the client and broker working.
 */
bool MqttClient::loop()
{
  // We use -1 for default 1000ms waiting for network activity.
	int result = mosquitto_loop(this->_data, -1, 1);

	return result == MOSQ_ERR_SUCCESS;
}


/*
 * Disconnect from server. It may be useful to get FALSE from
 * RaspberryOSIOClient::loop() and break from main communication cycle.
 */
bool MqttClient::disconnect()
{
	int result = mosquitto_disconnect(this->_data);
	return result = MOSQ_ERR_SUCCESS;
}
