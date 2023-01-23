#include "z5363966BaseRobot.hpp"

BaseRobot::BaseRobot()
    : Robot(),
      emitter(getEmitter("emitter")),
      receiver(getReceiver("receiver")),
      mGPS(getGPS("gps")),
      mCompass(getCompass("compass")),
      mLeftMotor(*getMotor("left wheel motor")),
      mRightMotor(*getMotor("right wheel motor")),
      robotName(getName()),
      state(IDLE),
      maxMotorSpeed(mLeftMotor.getMaxVelocity()),
      defaultMotorSpeed(0.5 * maxMotorSpeed),
      defaultMotorSpeedStep(0.1 * maxMotorSpeed),
      leftMotorDir(1),
      rightMotorDir(1),
      leftAbsMotorSpeed(0),
      rightAbsMotorSpeed(0),
      autoState(AUTO_IDLE),
      moveState(INTERMEDIATE_MOVE_IDLE),
      currentData(""),
      currentOrder(""),
      currentOrderPrice(0),
      purchaseConfirmation(false)

{
    robotID = (std::isdigit(robotName.back())) ? robotName.back() - '0' : 5;
    mKeyboard.enable(TIME_STEP);
    receiver->enable(TIME_STEP);
    mGPS->enable(TIME_STEP);
    mCompass->enable(TIME_STEP);
    assignBalance();
    setERChannels();
    step(TIME_STEP);
    startHeading = updateHeading();
    updatePosition();
    startXPos = currentX;
    startZPos = currentZ;
    // std::cout << "Robot " + std::to_string(robotID) + " has been created." << std::endl;
    std::cout << "Robot " + std::to_string(robotID) + "'s start position is " + std::to_string(startXPos) + std::to_string(startZPos) << std::endl;
    // std::cout << "Robot " + std::to_string(robotID) + "'s start heading is " + std::to_string(startHeading[0]) + " " + std::to_string(startHeading[1]) + " " + std::to_string(startHeading[2]) << std::endl;
}

void BaseRobot::move(double x, double y, double angle)
{
    double targetHeading = calculateHeadingToCoordinate(x, y);
    switch (moveState)
    {
    case INTERMEDIATE_MOVE_IDLE:
        moveState = INTERMEDIATE_MOVE_FACE;
        break;
    case INTERMEDIATE_MOVE_FACE:
        // if (checkPosition(x, y))
        // {
        //     moveState = INTERMEDIATE_MOVE_HEAD;
        //     std::cout << "MOVED!" << std::endl;
        //     return;
        // }
        // if (checkBearing(targetHeading))
        // {
        //     std::cout << "Moving forward!" << std::endl;
        //     movePosition(x, y, targetHeading);
        // }
        // else
        // {
        //     std::cout << "Turning!" << std::endl;
        //     moveHeading(targetHeading);
        // }
        if (!checkBearing(targetHeading))
        {
            moveHeading(targetHeading);
        }
        else
        {
            moveState = INTERMEDIATE_MOVE;
        }
        break;
    case INTERMEDIATE_MOVE:
        if (!checkPosition(x, y))
        {
            movePosition(x, y, targetHeading);
        }
        else
        {
            moveState = INTERMEDIATE_MOVE_HEAD;
        }
        break;
    case INTERMEDIATE_MOVE_HEAD:
        if (checkBearing(startHeading + angle * (180 / M_PI)))
        {
            moveState = INTERMEDIATE_MOVE_FINISH;
            std::cout << "DONE!" << std::endl;
            setMotorPosition();
            halt();
            setMotorSpeed();
        }
        else
        {
            moveHeading(startHeading + angle * (180 / M_PI));
        }
        break;
    case INTERMEDIATE_MOVE_FINISH:
        break;
    }
}

void BaseRobot::movePosition(double x, double z, double targetBearing)
{
    setMotorPosition();
    moveForward(maxMotorSpeed);
    setMotorSpeed();
}

void BaseRobot::moveHeading(double bearing)
{
    double delta = abs(currentHeading - bearing);
    // double speed = maxMotorSpeed * (delta * SPEED_MULTIPLIER + MINIMUM_MOTOR_SPEED);
    double speed = (delta < 4) ? linearAdjust(delta / 180) : maxMotorSpeed;
    setMotorPosition();
    if (bearing >= 90 && bearing <= 270)
        turnRight(speed);
    else
        turnLeft(speed);
    setMotorSpeed();
}

double BaseRobot::linearAdjust(double input)
{
    // return maxMotorSpeed * (input * SPEED_MULTIPLIER + MINIMUM_MOTOR_SPEED);
    // return maxMotorSpeed * (input * SPEED_MULTIPLIER);
    return 0.01 * maxMotorSpeed;
    // return maxMotorSpeed * MINIMUM_MOTOR_SPEED;
}

double BaseRobot::calculateHeadingToCoordinate(double targetX, double targetZ)
{
    double deltaX = targetX - currentX;
    double deltaZ = targetZ - currentZ;
    double rad = abs(atan2(deltaZ, deltaX));
    rad = (rad * 180) / M_PI;
    double bearing = (rad >= 0.0) ? rad : rad + 360.0;
    bearing += 180;
    // std::cout << "Required heading is: " + std::to_string(bearing) << std::endl;
    // std::cout << "Current heading is: " + std::to_string(currentHeading) << std::endl;
    return bearing;
}

bool BaseRobot::checkPosition(double x, double z)
{
    // std::cout << "Current position is " + std::to_string(currentX) + std::to_string(currentZ) << std::endl;
    return abs(currentX - x) < POSITION_TOLERANCE && abs(currentZ - z) < POSITION_TOLERANCE;
}

bool BaseRobot::checkBearing(double bearing)
{
    return abs(currentHeading - bearing) < BEARING_TOLERANCE;
}

double BaseRobot::updateHeading()
{
    const double *north = mCompass->getValues();
    double rad = atan2(north[2], north[0]);
    double bearing = (rad - 1.5708) / M_PI * 180.0;
    // bearing = (bearing >= 0.0) ? bearing : bearing + 360.0;
    // // std::cout << "Bearing: " + std::to_string(bearing) << std::endl;
    // return bearing;
    return (bearing >= 0.0) ? bearing : bearing + 360.0;
}

void BaseRobot::updatePosition()
{
    const double *gpsValues{mGPS->getValues()};
    currentX = gpsValues[0];
    currentZ = gpsValues[2];
    // std::copy(gpsValues, gpsValues + 3, currentPosition.begin());
    // std::cout << "Position: " + std::to_string(currentPosition[0]) + std::to_string(currentPosition[1]) + std::to_string(currentPosition[2]) << std::endl;
}

void BaseRobot::sendMessage(const std::string &str, int id)
{
    emitter->setChannel(id);
    emitter->send(str.data(), str.size() + 1);
}

std::string BaseRobot::receiveMessage()
{
    if (receiver->getQueueLength() != 0)
    {
        std::string data{(const char *)receiver->getData()};
        receiver->nextPacket();
        return data;
    }
    return "";
}

void BaseRobot::halt()
{
    leftAbsMotorSpeed = rightAbsMotorSpeed = 0;
}

void BaseRobot::moveForward(double speed)
{
    leftMotorDir = rightMotorDir = 1;
    leftAbsMotorSpeed = rightAbsMotorSpeed = speed;
}

void BaseRobot::moveBackward(double speed)
{
    leftMotorDir = rightMotorDir = -1;
    leftAbsMotorSpeed = rightAbsMotorSpeed = speed;
}

void BaseRobot::turnLeft(double speed)
{
    leftMotorDir = -1;
    rightMotorDir = 1;
    leftAbsMotorSpeed = rightAbsMotorSpeed = speed;
}

void BaseRobot::turnRight(double speed)
{
    leftMotorDir = 1;
    rightMotorDir = -1;
    leftAbsMotorSpeed = rightAbsMotorSpeed = speed;
}

void BaseRobot::increaseMotorSpeed()
{
    leftAbsMotorSpeed = rightAbsMotorSpeed += defaultMotorSpeedStep;
    if (leftAbsMotorSpeed > maxMotorSpeed)
    {
        leftAbsMotorSpeed = maxMotorSpeed;
    }
    if (rightAbsMotorSpeed > maxMotorSpeed)
    {
        rightAbsMotorSpeed = maxMotorSpeed;
    }
}

void BaseRobot::decreaseMotorSpeed()
{
    leftAbsMotorSpeed = rightAbsMotorSpeed -= defaultMotorSpeedStep;
    if (leftAbsMotorSpeed < 0)
    {
        leftAbsMotorSpeed = 0;
    }
    if (rightAbsMotorSpeed < 0)
    {
        rightAbsMotorSpeed = 0;
    }
}

void BaseRobot::setMotorPosition()
{
    mLeftMotor.setPosition(INFINITY);
    mRightMotor.setPosition(INFINITY);
}

void BaseRobot::setMotorSpeed()
{
    mLeftMotor.setVelocity(leftMotorDir * leftAbsMotorSpeed);
    mRightMotor.setVelocity(rightMotorDir * rightAbsMotorSpeed);
}

void BaseRobot::setERChannels()
{
    receiver->setChannel(robotID);
    emitter->setChannel(5);
}

void BaseRobot::assignBalance()
{
    std::ifstream startingFile("../../Starting.csv", std::ifstream::in);
    std::string lineInput;
    while (std::getline(startingFile, lineInput))
    {
        if (lineInput[0] - '0' == robotID)
        {
            // Assigns balance to the appropriate robot
            mBalance = std::stod(lineInput.substr(2, lineInput.size() - 2));
            // std::cout << "Customer " + std::to_string(robotID) + "'s balance is " + std::to_string(mBalance) << std::endl;
            break;
        }
    }
}

void BaseRobot::printBalance()
{
    std::cout << robotName + ": My current balance is " << std::setprecision(2) << std::fixed << mBalance << std::endl;
}

BaseRobot::~BaseRobot() {}
