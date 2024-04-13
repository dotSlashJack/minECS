#include "StateSequenceRunner.h"

std::map<std::string, int> PinMappings = {
    {"gox_press", 27},
    {"gox_vent", 19},
    {"gox_flow", 26},
    {"gox_purge", 23},
    {"ker_press", 22},
    {"ker_vent", 21},
    {"ker_flow", 20},
    {"ker_purge", 24}
};


GPIO goxPressPin(PinMappings["gox_press"]);
GPIO goxVentPin(PinMappings["gox_vent"]);
GPIO goxFlowPin(PinMappings["gox_flow"]);
GPIO goxPurgePin(PinMappings["gox_purge"]);
GPIO kerPressPin(PinMappings["ker_press"]);
GPIO kerVentPin(PinMappings["ker_vent"]);
GPIO kerFlowPin(PinMappings["ker_flow"]);
GPIO kerPurgePin(PinMappings["ker_purge"]);

map<string, map<string, string>> StateMaps = {
    {"ONLINE_SAFE", {
        {"gox_press", "HIGH"},
        {"gox_vent", "HIGH"},
        {"gox_flow", "HIGH"},
        {"gox_purge", "HIGH"},
        {"ker_press", "HIGH"},
        {"ker_vent", "HIGH"},
        {"ker_flow", "HIGH"},
        {"ker_purge", "HIGH"}
    }},
    {"ALL_OFF", {
        {"gox_press", "LOW"},
        {"gox_vent", "LOW"},
        {"gox_flow", "LOW"},
        {"gox_purge", "LOW"},
        {"ker_press", "LOW"},
        {"ker_vent", "LOW"},
        {"ker_flow", "LOW"},
        {"ker_purge", "LOW"}
    }}
};

SequencesMap StateSequenceRunner::sequences = {
    {"Sequence1", {
        {milliseconds(1000), "ONLINE_SAFE"},
        {milliseconds(2000), "ONLINE_SAFE"},
        {milliseconds(1500), "ONLINE_SAFE"}
    }}
};

// TODO in this change the order of high vs low based on open/closed actual states
void StateSequenceRunner::executeState(const string& stateName) {
    cout << "Executing state: " << stateName << endl;

    // Find the state in the StateMaps
    if (StateMaps.count(stateName) > 0) {
        const auto& stateMap = StateMaps.at(stateName);
        for (const auto& [pinName, value] : stateMap) {
            cout << "Processing pin: " << pinName << " with value: " << value << endl;

            // Set the GPIO pin state based on the state value
            if (pinName == "gox_press") {
                cout << "Setting gox_press pin" << endl;
                value == "HIGH" ? goxPressPin.setHigh() : goxPressPin.setLow();
                cout << "gox_press pin set to" + value << endl;
            } else if (pinName == "gox_vent") {
                cout << "Setting gox_vent pin" << endl;
                value == "HIGH" ? goxVentPin.setHigh() : goxVentPin.setLow();
                cout << "gox_vent pin set to" + value << endl;
            } else if (pinName == "gox_flow") {
                cout << "Setting gox_flow pin to" + value << endl;
                value == "HIGH" ? goxFlowPin.setHigh() : goxFlowPin.setLow();
                cout << "gox_flow pin set to" + value << endl;
            } else if (pinName == "gox_purge") {
                cout << "Setting gox_purge pin to" + value << endl;
                value == "HIGH" ? goxPurgePin.setHigh() : goxPurgePin.setLow();
                cout << "gox_purge pin set to" + value << endl;
            } else if (pinName == "ker_press") {
                cout << "Setting ker_press pinto" + value << endl;
                value == "HIGH" ? kerPressPin.setHigh() : kerPressPin.setLow();
                cout << "ker_press pin setto" + value << endl;
            } else if (pinName == "ker_vent") {
                cout << "Setting ker_vent pinto" + value << endl;
                value == "HIGH" ? kerVentPin.setHigh() : kerVentPin.setLow();
                cout << "ker_vent pin setto" + value << endl;
            } else if (pinName == "ker_flow") {
                cout << "Setting ker_flow pinto" + value << endl;
                value == "HIGH" ? kerFlowPin.setHigh() : kerFlowPin.setLow();
                cout << "ker_flow pin set to" + value << endl;
            } else if (pinName == "ker_purge") {
                cout << "Setting ker_purge pin to" + value << endl;
                value == "HIGH" ? kerPurgePin.setHigh() : kerPurgePin.setLow();
                cout << "ker_purge pin setto" + value << endl;
            } else {
                cout << "Pin not found: " << pinName << endl;
            }
        }
    } else {
        cout << "State not found: " << stateName << endl;
    }

    cout << "State execution completed: " << stateName << endl;
}

void StateSequenceRunner::executeSequence(const Sequence& sequence) {
    for (const auto& step : sequence) {
        if (abortFlag) {
            cout << "Sequence aborted. Setting state to ONLINE_SAFE." << endl;
            executeState("ONLINE_SAFE");
            abortFlag = false;
            return;
        }

        executeState(step.second);
        // Wait for the specified duration
        this_thread::sleep_for(step.first);
    }
}

string StateSequenceRunner::sequenceRunner(string sequenceKey) {
    if (sequences.count(sequenceKey) > 0) {
        const auto& sequence = sequences[sequenceKey];
        executeSequence(sequence);
    } else {
        cout << "Sequence not found: " << sequenceKey << endl;
    }
    return "Executed sequence: " + sequenceKey;
}

void StateSequenceRunner::abortSequence() {
    abortFlag = true;
}