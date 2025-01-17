/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   EntityType.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 14 de Agosto de 2018, 19:24
 */

#include "EntityType.h"
#include "Model.h"
#include "SimulationResponse.h"

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &EntityType::GetPluginInformation;
}
#endif

ModelDataDefinition* EntityType::NewInstance(Model* model, std::string name) {
	return new EntityType(model, name);
}

EntityType::EntityType(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<EntityType>(), name) {
}

void EntityType::_initBetweenReplications() {
	ModelDataDefinition::_initBetweenReplications();
	_initialWaitingCost = 0.0;
	_initialVACost = 0.0;
	_initialNVACost = 0.0;
	_initialOtherCost = 0.0;
	//for (StatisticsCollector* cstat : *_statisticsCollectors->list()) {
	//	cstat->getStatistics()->getCollector()->clear();
	//}
}

EntityType::~EntityType() {
	// remove all CStats
	for (StatisticsCollector* cstat : *_statisticsCollectors->list()) {
		_parentModel->getDataManager()->remove(Util::TypeOf<StatisticsCollector>(), cstat);
	}
}

std::string EntityType::show() {
	return ModelDataDefinition::show() +
			",initialPicture=" + this->_initialPicture; // add more...
}

void EntityType::setInitialWaitingCost(double _initialWaitingCost) {
	this->_initialWaitingCost = _initialWaitingCost;
}

double EntityType::initialWaitingCost() const {
	return _initialWaitingCost;
}

void EntityType::setInitialOtherCost(double _initialOtherCost) {
	this->_initialOtherCost = _initialOtherCost;
}

double EntityType::initialOtherCost() const {
	return _initialOtherCost;
}

void EntityType::setInitialNVACost(double _initialNVACost) {
	this->_initialNVACost = _initialNVACost;
}

double EntityType::initialNVACost() const {
	return _initialNVACost;
}

void EntityType::setInitialVACost(double _initialVACost) {
	this->_initialVACost = _initialVACost;
}

double EntityType::initialVACost() const {
	return _initialVACost;
}

void EntityType::setInitialPicture(std::string _initialPicture) {
	this->_initialPicture = _initialPicture;
}

std::string EntityType::initialPicture() const {
	return _initialPicture;
}

StatisticsCollector* EntityType::addGetStatisticsCollector(std::string name) {
	for (StatisticsCollector* cstat : *_statisticsCollectors->list()) {
		if (cstat->getName() == name) {
			return cstat;
		}
	}
	// not found. Create it, insert it into the list of cstats, into the model modeldatum manager, and then return it
	StatisticsCollector* cstat = new StatisticsCollector(_parentModel, name, this);
	_statisticsCollectors->insert(cstat); // @TODO _statisticsCollectors list is probabily redundant to _internelElements and unnecessary
	_internalData->insert({name, cstat});
	//_parentModel->insert(cstat); // unnecessary
	return cstat;
}

PluginInformation* EntityType::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<EntityType>(), &EntityType::LoadInstance, &EntityType::NewInstance);
	info->setDescriptionHelp("//@TODO");
	return info;
}

ModelDataDefinition* EntityType::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	EntityType* newElement = new EntityType(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool EntityType::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		this->_initialNVACost = LoadField(fields, "initialNVACost", DEFAULT.initialCost);
		this->_initialOtherCost = LoadField(fields, "initialOtherCost", DEFAULT.initialCost);
		this->_initialVACost = LoadField(fields, "initialVACost", DEFAULT.initialCost);
		this->_initialWaitingCost = LoadField(fields, "initialWaitingCost", DEFAULT.initialCost);
		this->_initialPicture = LoadField(fields, "initialPicture", DEFAULT.initialPicture);
	}
	return res;
}

std::map<std::string, std::string>* EntityType::_saveInstance(bool saveDefaultValues) {
	bool saveDefaults = _getSaveDefaultsOption();
	std::map<std::string, std::string>* fields = ModelDataDefinition::_saveInstance(saveDefaultValues); //Util::TypeOf<EntityType>());
	SaveField(fields, "initialNVACost", _initialNVACost, DEFAULT.initialCost, saveDefaults);
	SaveField(fields, "initialOtherCost", _initialOtherCost, DEFAULT.initialCost, saveDefaults);
	SaveField(fields, "initialVACost", _initialVACost, DEFAULT.initialCost, saveDefaults);
	SaveField(fields, "initialWaitingCost", _initialWaitingCost, DEFAULT.initialCost, saveDefaults);
	SaveField(fields, "initialPicture", _initialPicture, DEFAULT.initialPicture, saveDefaults);
	return fields;
}

bool EntityType::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}

void EntityType::_createInternalData() {
	if (_reportStatistics) { //@TODO: Fix inserting to _internalElements
	} else {
		while (_statisticsCollectors->size() > 0) {
			_parentModel->getDataManager()->remove(_statisticsCollectors->front());
			_statisticsCollectors->front()->~StatisticsCollector();
			_statisticsCollectors->pop_front();
		}
	}
}

