#pragma once
#include <vector>
#include "util.hpp"
#include <jansson.h>


namespace rack {


struct Param {
	float value = 0.0;
};

struct Light {
	/** The square of the brightness value */
	float value = 0.0;
	float getBrightness();
	void setBrightness(float brightness) {
		value = brightness * brightness;
	}
	void setBrightnessSmooth(float brightness);
};

struct Wire;

struct Input {
	/** Voltage of the port, zero if not plugged in. Read-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
	Wire *wire = NULL;
	Light plugLights[2];
	/** Returns the value if a wire is plugged in, otherwise returns the given default value */
	float normalize(float normalValue) {
		return active ? value : normalValue;
	}
};

struct Output {
	/** Voltage of the port. Write-only by Module */
	float value = 0.0;
	/** Whether a wire is plugged in */
	bool active = false;
	Wire *wire = NULL;
	Light plugLights[2];
};


struct Module {
	std::vector<Param> params;
	std::vector<Input> inputs;
	std::vector<Output> outputs;
	std::vector<Light> lights;
	/** For CPU usage meter */
	float cpuTime = 0.0;

	/** Deprecated, use constructor below this one */
	Module() DEPRECATED {}
	/** Constructs Module with a fixed number of params, inputs, and outputs */
	Module(int numParams, int numInputs, int numOutputs, int numLights = 0) {
		params.resize(numParams);
		inputs.resize(numInputs);
		outputs.resize(numOutputs);
		lights.resize(numLights);
	}
	virtual ~Module() {}

	/** Advances the module by 1 audio frame with duration 1.0 / gSampleRate */
	virtual void step() {}
	virtual void stepStream(const float *input, float *output, int numFrames) {}
	virtual void onSampleRateChange() {}

	std::vector<Module*> inputModules;
	
	/**
	   Override this to handle the event that a module was added as an input.
	   Example usage in src/core/AudioInterface.cpp enables AudioInterface to
	   walk the modules connected to its inputs to render the audio using stepStream.
	*/
	void onModuleInput(Module *module) {
		inputModules.push_back(module);
	}

	/** Override these to implement spacial behavior when user clicks Initialize and Randomize */
	virtual void reset() {}
	virtual void randomize() {}
	/** Deprecated */
	virtual void initialize() final {}

	/** Override these to store extra internal data in the "data" property */
	virtual json_t *toJson() { return NULL; }
	virtual void fromJson(json_t *root) {}
};

struct Wire {
	Module *outputModule = NULL;
	int outputId;
	Module *inputModule = NULL;
	int inputId;
	void step();
};

void engineInit();
void engineDestroy();
/** Launches engine thread */
void engineStart();
void engineStop();
/** Does not transfer pointer ownership */
void engineAddModule(Module *module);
void engineRemoveModule(Module *module);
/** Does not transfer pointer ownership */
void engineAddWire(Wire *wire);
void engineRemoveWire(Wire *wire);
void engineSetParam(Module *module, int paramId, float value);
void engineSetParamSmooth(Module *module, int paramId, float value);
void engineSetSampleRate(float sampleRate);
float engineGetSampleRate();
/** Returns the inverse of the current sample rate */
float engineGetSampleTime();

extern bool gPaused;


} // namespace rack
