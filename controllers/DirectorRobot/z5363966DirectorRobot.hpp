#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <webots/Robot.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>

class DirectorRobot
{
public:
    DirectorRobot();
    void printCommandMenu();
    void menuSelect(int key);
    void startRemoteControl(int key);
    void startAutoMode();
    void autoMode();

    void run();
    
    ~DirectorRobot();
private:
    webots::Robot *robot;
    webots::Keyboard mKeyboard;
    webots::Emitter *emitter;
    webots::Receiver *receiver;

    int state;
    std::vector<char> allowedRemoteCommands;

    std::ifstream orderFile;
    std::string lineInput;
    std::string currentOrder;
    int currentCustomer;
    int orderCounter;

    // Constants
    static constexpr int TIME_STEP {64};

    // States
    static constexpr int INITIAL {0};
    static constexpr int REMOTE_CONTROL_INITIALISE {1};
    static constexpr int REMOTE {2};
    static constexpr int AUTO_INITIALISE {3};
    static constexpr int AUTO {4};
    static constexpr int AUTO_IDLE {5};
    static constexpr int END {6};
};