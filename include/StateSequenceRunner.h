#ifndef STATE_SEQUENCE_RUNNER_H
#define STATE_SEQUENCE_RUNNER_H

#include "GPIO.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <vector>
#include <atomic>

using namespace std;
using namespace std::chrono;

using Duration = milliseconds;
using StateMap = map<string, string>;
using SequenceStep = pair<Duration, string>;
using Sequence = vector<SequenceStep>;
using SequencesMap = map<string, Sequence>;

// number is the GPIO pin on the pi associated with each valve on the relay board
extern std::map<std::string, int> PinMappings;

// generate the actual GPIO objects
extern GPIO goxPressPin;
extern GPIO goxVentPin;
extern GPIO goxFlowPin;
extern GPIO goxPurgePin;
extern GPIO kerPressPin;
extern GPIO kerVentPin;
extern GPIO kerFlowPin;
extern GPIO kerPurgePin;

extern map<string, map<string, string>> StateMaps;
//extern SequencesMap sequences;

class StateSequenceRunner {
public:
    string sequenceRunner(string sequenceKey);
    void executeState(const string& stateName);
    void abortSequence();

    static SequencesMap sequences;

private:
    void executeSequence(const Sequence& sequence);
    std::atomic<bool> abortFlag{false};
};

#endif // STATE_SEQUENCE_RUNNER_H