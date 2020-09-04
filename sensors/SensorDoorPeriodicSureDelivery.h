/*
* The MySensors Arduino library handles the wireless radio link and protocol
* between your home built sensors/actuators and HA controller of choice.
* The sensors forms a self healing radio network with optional repeaters. Each
* repeater and gateway builds a routing tables in EEPROM which keeps track of the
* network topology allowing messages to be routed to nodes.
*
* Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
* Copyright (C) 2013-2017 Sensnology AB
* Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
*
* Documentation: http://www.mysensors.org
* Support Forum: http://forum.mysensors.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*/
#ifndef SensorDoorPeriodicSureDelivery_h
#define SensorDoorPeriodicSureDelivery_h

/*
* SensorDoorPeriodicSureDelivery
* This sensor require some additinal settings that need to be add to work this properly:
	nodeManager.setAck(true); //Has to be set! Without this it has no sense 
	nodeManager.setRetries(3); // Set retries count
  	nodeManager.setSleepBetweenRetries(3000); //SleepOrWait time between retries in miliseconds
  	nodeManager.setSleepBetweenRetriesSleepOrWait(true); //If sleep instead of wait, recomended is true	  
*/

#include <sensors/SensorInterrupt.h>

class SensorDoorPeriodicSureDelivery: public SensorInterrupt {
private:
	bool _goingFromInterrupt = false;
	uint8_t _retries = 10;
	unsigned int _sleep_between_retries = 5000;
	bool _sleep_between_retries_sleep_or_wait = true;
	//Only change this default values if you change them for nodeManager, so when this sensor change tham back, they
	//are same. This needs to be done manually from sketch file. 
	// These pre-set values are same as are default nodeManager values
	uint8_t _default_retries = 1;
	unsigned int _default_sleep_between_retries = 0;
	bool _default_sleep_between_retries_sleep_or_wait = false;
	bool _default_ack = false;

	//if request ECHO and set Retryies during standard loop also. Defaul is false, so request ECHO only in interrrupt
	bool _sure_deli_on_loop = false;

public:
	SensorDoorPeriodicSureDelivery(int8_t pin, uint8_t child_id = 0): SensorInterrupt(pin, child_id) {
		_name = "DOOR_PERIODIC_SURE_DELI";
		children.get()->setPresentation(S_DOOR);
		children.get()->setType(V_TRIPPED);
		children.get()->setDescription(_name);
	};

	// setter/getter
	void setRetries(uint8_t value) {
		_retries = value;
	};
	void setSleepBetweenRetries(unsigned int value) {
		_sleep_between_retries = value;
	};
  	void setSleepBetweenRetriesSleepOrWait(bool value) {
		_sleep_between_retries_sleep_or_wait = value;
	};
	// default vaules that are set for nodeManager
	void setDefaultRetries(uint8_t value) {
		_default_retries = value;
	};
	void setDefaultSleepBetweenRetries(unsigned int value) {
		_default_sleep_between_retries = value;
	};
  	void setDefaultSleepBetweenRetriesSleepOrWait(bool value) {
		_default_sleep_between_retries_sleep_or_wait = value;
	};
	void setDefaultAck(bool value){
		_default_ack = value;
	}
	void setSureDeliveryOnLoop(bool value){
		_sure_deli_on_loop = value;
	};

	// define what to do during loop
	void onLoop(Child* child) {
		if(_goingFromInterrupt == false){
			//if sure delivery is requested for standard report interval
			if (_sure_deli_on_loop == true) startSureDelivery();
			// read the value
			int value = digitalRead(_pin);
			// invert the value if needed
			if (_invert_value_to_report) value = !value;
			// store the value
			child->setValue(value);
		}
		_goingFromInterrupt = false;
	};

	// what to do when receiving an interrupt
	void onInterrupt() {
		_goingFromInterrupt = true;
		startSureDelivery();
		SensorInterrupt::onInterrupt();
	};

	// what to do as the main task when receiving a message
	void onReceive(MyMessage* message) {
		Child* child = getChild(message->sensor);
		if (child == nullptr) return;

		if (message->getCommand() == C_REQ && message->type == V_STATUS) {
			// return current status
			child->setValue(digitalRead(_pin));
		}
		else if(message->isEcho() == true && 
				message->getSensor() == child->getChildId() && 
				message->getCommand() == C_SET && 
				message->type == V_TRIPPED 
				) {
			//Skip remianing retries
			nodeManager.stopRetryingNow();
		}
	};

	//after send, wheter it was successsfully echo-ed or not, set default values
	void afterSend(Child* child){
		stopSureDelivery();
	}

private:
	void startSureDelivery(){
		nodeManager.setRetries(_retries);
  		nodeManager.setSleepBetweenRetries(_sleep_between_retries);
  		nodeManager.setSleepBetweenRetriesSleepOrWait(_sleep_between_retries_sleep_or_wait);
		nodeManager.setAck(true);
	};
	void stopSureDelivery(){
		nodeManager.setRetries(_default_retries);
  		nodeManager.setSleepBetweenRetries(_default_sleep_between_retries);
  		nodeManager.setSleepBetweenRetriesSleepOrWait(_default_sleep_between_retries_sleep_or_wait);
		nodeManager.setAck(_default_ack);
	};
};

#endif