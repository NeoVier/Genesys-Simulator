/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Resource.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Agosto de 2018, 16:52
 */

#include "Resource.h"
#include "../../kernel/simulator/Counter.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC 

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Resource::GetPluginInformation;
}
#endif

ModelDataDefinition* Resource::NewInstance(Model* model, std::string name) {
	return new Resource(model, name);
}

Resource::Resource(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Resource>(), name) {
	_resourceEventHandlers->setSortFunc([](const SortedResourceEventHandler* a, const SortedResourceEventHandler * b) {
		return a->second < b->second; /// Handlers are sorted by priority
	});

	GetterMember getter = DefineGetterMember<Resource>(this, &Resource::getCapacity);
	SetterMember setter = DefineSetterMember<Resource>(this, &Resource::setCapacity);
	model->getControls()->insert(new SimulationControl(Util::TypeOf<Resource>(), getName() + ".Capacity", getter, setter));

	GetterMember getter2 = DefineGetterMember<Resource>(this, &Resource::getCostPerUse);
	SetterMember setter2 = DefineSetterMember<Resource>(this, &Resource::setCostPerUse);
	model->getControls()->insert(new SimulationControl(Util::TypeOf<Resource>(), getName() + ".CostPerUse", getter2, setter2));
	// ...
}

std::string Resource::show() {
	return ModelDataDefinition::show() +
			",capacity=" + strTruncIfInt(std::to_string(_capacity)) +
			",costBusyByour=" + strTruncIfInt(std::to_string(_costBusyHour)) +
			",costIdleByour=" + strTruncIfInt(std::to_string(_costIdleHour)) +
			",costPerUse=" + strTruncIfInt(std::to_string(_costPerUse)) +
			",state=" + strTruncIfInt(std::to_string(static_cast<int> (_resourceState)));
}

void Resource::seize(unsigned int quantity) {
	_numberBusy += quantity;
	if (_reportStatistics)
		_numSeizes->incCountValue(quantity);
	_lastTimeSeized = _parentModel->getSimulation()->getSimulatedTime();
	_resourceState = Resource::ResourceState::BUSY;
	// @TODO implement costs
}

void Resource::release(unsigned int quantity) {
	if (_numberBusy >= quantity) {
		_numberBusy -= quantity;
	} else {
		_numberBusy = 0;
	}
	if (_numberBusy == 0) {
		_resourceState = Resource::ResourceState::IDLE;
	}
	double timeSeized = _parentModel->getSimulation()->getSimulatedTime() - _lastTimeSeized;
	if (_reportStatistics) {
		_numReleases->incCountValue(quantity);
		_cstatTimeSeized->getStatistics()->getCollector()->addValue(timeSeized);
		_totalTimeSeized->incCountValue(timeSeized);
	}
	//
	_lastTimeSeized = timeSeized;
	_notifyReleaseEventHandlers();
}

void Resource::_initBetweenReplications() {
	ModelDataDefinition::_initBetweenReplications();
	this->_lastTimeSeized = 0.0;
	this->_numberBusy = 0;
}

void Resource::setResourceState(ResourceState _resourceState) {
	this->_resourceState = _resourceState;
}

Resource::ResourceState Resource::getResourceState() const {
	return _resourceState;
}

void Resource::setCapacity(unsigned int _capacity) {
	this->_capacity = _capacity;
}

unsigned int Resource::getCapacity() const {
	return _capacity;
}

void Resource::setCostBusyHour(double _costBusyHour) {
	this->_costBusyHour = _costBusyHour;
}

double Resource::getCostBusyHour() const {
	return _costBusyHour;
}

void Resource::setCostIdleHour(double _costIdleHour) {
	this->_costIdleHour = _costIdleHour;
}

double Resource::getCostIdleHour() const {
	return _costIdleHour;
}

void Resource::setCostPerUse(double _costPerUse) {
	this->_costPerUse = _costPerUse;
}

double Resource::getCostPerUse() const {
	return _costPerUse;
}

unsigned int Resource::getNumberBusy() const {
	return _numberBusy;
}

void Resource::addReleaseResourceEventHandler(ResourceEventHandler eventHandler, ModelComponent* component, unsigned int priority) {
	//if (_resourceEventHandlers->find(eventHandler) != _resourceEventHandlers->list()->end()) {
	ModelComponent* compHandler;
	for (SortedResourceEventHandler* sreh : *_resourceEventHandlers->list()) {
		compHandler = sreh->first.second;
		if (compHandler == component) {
			return; // already exists. Do not insert again
		}
	}
	SortedResourceEventHandler* sortedEventHandler = new SortedResourceEventHandler({
		{eventHandler, component}, priority
	});
	_resourceEventHandlers->insert(sortedEventHandler);
}

double Resource::getLastTimeSeized() const {
	return _lastTimeSeized;
}

void Resource::_notifyReleaseEventHandlers() {
	for (SortedResourceEventHandler* sortedHandler : *_resourceEventHandlers->list()) {
		ResourceEventHandler handler = sortedHandler->first.first;
		handler(this);
	}
}

PluginInformation* Resource::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Resource>(), &Resource::LoadInstance, &Resource::NewInstance);
	return info;
}

ModelDataDefinition* Resource::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Resource* newElement = new Resource(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Resource::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_capacity = LoadField(fields, "capacity", DEFAULT.capacity);
		_costBusyHour = LoadField(fields, "costBusyHour", DEFAULT.cost);
		_costIdleHour = LoadField(fields, "costIdleHour", DEFAULT.cost);
		_costPerUse = LoadField(fields, "costPerUse", DEFAULT.cost);
		_resourceState = static_cast<Resource::ResourceState> (LoadField(fields, "resourceState", static_cast<int> (DEFAULT.resourceState)));
	}
	return res;
}

std::map<std::string, std::string>* Resource::_saveInstance(bool saveDefaultValues) {
	bool saveDefaults = _getSaveDefaultsOption();
	std::map<std::string, std::string>* fields = ModelDataDefinition::_saveInstance(saveDefaultValues); //Util::TypeOf<Resource>());
	SaveField(fields, "capacity", _capacity, DEFAULT.capacity, saveDefaultValues);
	SaveField(fields, "costBusyHour", _costBusyHour, DEFAULT.cost, saveDefaultValues);
	SaveField(fields, "costIdleHour", _costIdleHour, DEFAULT.cost, saveDefaultValues);
	SaveField(fields, "costPerUse", _costPerUse, DEFAULT.cost, saveDefaultValues);
	SaveField(fields, "resourceState", static_cast<int> (_resourceState), static_cast<int> (DEFAULT.resourceState), saveDefaultValues);
	return fields;
}

bool Resource::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}

void Resource::_createInternalData() {
	if (_reportStatistics && _cstatTimeSeized == nullptr) {
		_cstatTimeSeized = new StatisticsCollector(_parentModel, getName() + "." + "TimeSeized", this);
		_totalTimeSeized = new Counter(_parentModel, getName() + "." + "TotalTimeSeized", this);
		_numSeizes = new Counter(_parentModel, getName() + "." + "Seizes", this);
		_numReleases = new Counter(_parentModel, getName() + "." + "Releases", this);
		_internalData->insert({"TimeSeized", _cstatTimeSeized});
		_internalData->insert({"TotalTimeSeized", _totalTimeSeized});
		_internalData->insert({"Seizes", _numSeizes});
		_internalData->insert({"Releases", _numReleases});
	} else if (!_reportStatistics && _cstatTimeSeized != nullptr) {
		_removeInternalDatas();
	}
}