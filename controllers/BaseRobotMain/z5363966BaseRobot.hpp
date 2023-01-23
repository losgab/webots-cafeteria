#include <iostream>
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// Math
#define _USE_MATH_DEFINES
#include <cmath>

#include <webots/Robot.hpp>
#include <webots/Keyboard.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>
#include <webots/Motor.hpp>
#include <webots/GPS.hpp>
#include <webots/Compass.hpp>

class BaseRobot : public webots::Robot {
    public:
        /**
         * @brief Constructs a new base robot
         * 
         */
        BaseRobot();
        
        /**
         * @brief Move to the input location. The inputs are x, y, heading in [meter, meter, radian]. The
         * robot should move to the position x, y, and turn heading radians relative to the starting
         * position and heading of the robot.
         * 
         */
        void move(double, double, double);

        /**
         * @brief Moves robot to target coordinates
         * 
         * 
         * @param x, y, targetBearing
        */
        void movePosition(double, double, double);

        /**
         * @brief Moves robot to target heading
         * 
         * @param bearing [0, 360)
        */
        void moveHeading(double);

        /**
         * @brief Moves robot to target heading
         * 
         * @param delta
         * 
         * @return double - linearised motor speed
        */
        double linearAdjust(double);

        /**
         * @brief Calculates the heading required to face the target position
         * 
         * @return double
        */
        double calculateHeadingToCoordinate(double, double);

        /**
         * @brief checks whether the robot has arrived at target coordinates
         * 
         * @param double x
         * @param double z
         * 
         * @return boolean
        */
        bool checkPosition(double, double);

        /**
         * @brief checks whether the robot is facing target bearing
         * 
         * @param bearing degrees
         * 
         * @return boolean, true if within tolerance
        */
        bool checkBearing(double);

        /**
         * @brief Updates the current heading of the robot
         * 
         * @return double
         */
        double updateHeading();
        
        /**
         * @brief Updates the current position of the robot
         * 
         */
        void updatePosition();
        
        /**
         * @brief Send a message string to the robot. The inputs are message string and the robotID
         * 
         */
        void sendMessage(const std::string&, int);

        /**
         * @brief Receive the message from other robots and return the message. If no message received, return
         * an empty string.
         * 
         * @return std::string 
         */
        std::string receiveMessage();

        /**
         * @brief Stops all movement
         * 
         */
        void halt();

        /**
         * @brief Makes both motors move forward
         * 
         */
        void moveForward(double);

        /**
         * @brief Makes both motors move backward
         * 
         */
        void moveBackward(double);

        /**
         * @brief Makes motors spin in opposite directions to turn left
         * 
         */
        void turnLeft(double);

        /**
         * @brief Makes motors spin in oppposite directions to turn right
         * 
         */
        void turnRight(double);

        /**
         * @brief Increase absolute motor speeds by 0.1 * MaxMotorSpeed
         * 
         */
        void increaseMotorSpeed();

        /**
         * @brief Decrease absolute motor speeds by 0.1 * MaxMotorSpeed
         * 
         */
        void decreaseMotorSpeed();

        /**
         * @brief Set the Motor Position 
         * 
         */
        void setMotorPosition();

        /**
         * @brief Set the Motor Speed 
         * 
         */
        void setMotorSpeed();

        /**
         * @brief Set the Emitter/Receiver Channels for all customer and staff robots
         * 
         */
        void setERChannels();

        /**
         * @brief Processes data if available
         * 
         */
        virtual void processData() = 0;

        /**
         * @brief Prints the state of the robot in meters and radians.
         * The position and orientation of the robot should round to 3 decimal places.
         * 
         * @return std::ostream& 
         */
        friend std::ostream& operator<<(std::ostream&, BaseRobot const&);

        virtual void run() = 0;
        virtual void remoteControl() = 0;
        virtual void autoMode() = 0;

        /**
         * @brief Resets all auto ordering fields
         * 
         */
        virtual void resetOrdering() = 0;

        /**
         * @brief Assigns the robot its starting balance
         * 
         */
        void assignBalance();

        /**
         * @brief Reports on the customer's current balance
         * 
         */
        void printBalance();

        /**
         * @brief Destroy the Base Robot object
         * 
         */
        ~BaseRobot();
    protected:
        webots::Emitter *emitter;
        webots::Receiver *receiver;
        webots::Keyboard mKeyboard;
        webots::GPS *mGPS;
        webots::Compass *mCompass;
        webots::Motor& mLeftMotor;
        webots::Motor& mRightMotor;

        int robotID;
        std::string robotName;

        int state;
        int currentKey;

        // Motor control fields
        double maxMotorSpeed;
        double defaultMotorSpeed;
        double defaultMotorSpeedStep;

        // 1 is forward (positive), -1 is backward (negative)
        int leftMotorDir;
        int rightMotorDir;

        // Keeps track of motor speeds
        double leftAbsMotorSpeed;
        double rightAbsMotorSpeed;    

        // Auto fields
        double mBalance;
        int autoState;
        int moveState;
        std::string currentData;
        std::string currentOrder;
        double currentOrderPrice;
        bool purchaseConfirmation;

        // Movement fields
        // std::array<double, 3> currentPosition;
        double startHeading;
        double startXPos;
        double startZPos;
        double currentX;
        double currentZ;
        double currentHeading;

        static constexpr int TIME_STEP {64};

        // Control States
        static constexpr int IDLE {0};
        static constexpr int REMOTE {2};
        static constexpr int AUTO {4};
        static constexpr int END {6};

        // Auto States
        static constexpr int AUTO_IDLE {10};
        static constexpr int AUTO_MOVE_ORDER_COUNTER {20};
        static constexpr int AUTO_MOVE_PICKUP_COUNTER {30};
        static constexpr int AUTO_MOVE_STARTING_POSITION {40};

        // Movement states
        static constexpr int INTERMEDIATE_MOVE_IDLE {50};
        static constexpr int INTERMEDIATE_MOVE_FACE {60};
        static constexpr int INTERMEDIATE_MOVE {70};
        static constexpr int INTERMEDIATE_MOVE_HEAD {80};
        static constexpr int INTERMEDIATE_MOVE_FINISH {0};

        // Robot stats
        static constexpr double AXLE_LENGTH {0.045};
        static constexpr double WHEEL_RADIUS {0.025};

        // Movement Control
        static constexpr double POSITION_TOLERANCE {0.01};
        static constexpr double BEARING_TOLERANCE {0.05};
        static constexpr double SPEED_MULTIPLIER {1.05};
        static constexpr double MINIMUM_MOTOR_SPEED {0.1};
};

// std::ostream& operator<<(std::ostream& out, BaseRobot const& br) {
//     return;
// }