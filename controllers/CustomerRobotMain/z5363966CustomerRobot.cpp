#include "z5363966CustomerRobot.hpp"

CustomerRobot::CustomerRobot()
    : BaseRobot(),
      currentOrderItemExists(false),
      orderReady(false) {}

void CustomerRobot::run()
{
    // Initial mode: waits for what mode to enter into

    while (step(TIME_STEP) != -1)
    {
        this->currentKey = mKeyboard.getKey();

        // Find when a message is received to go into auto or remote mode
        currentData = receiveMessage();
        currentHeading = updateHeading();
        updatePosition();
        processData();

        switch (state)
        {
        case REMOTE:
            remoteControl();
            break;
        case AUTO:
            autoMode();
            break;
        case END:
            return;
        }
    }
}

void CustomerRobot::remoteControl()
{
    if (currentKey != EOF)
    {
        setMotorPosition();
        switch (std::tolower(currentKey))
        {
        case 'w':
            moveForward(defaultMotorSpeed);
            break;
        case 'a':
            turnLeft(defaultMotorSpeed);
            break;
        case 's':
            moveBackward(defaultMotorSpeed);
            break;
        case 'd':
            turnRight(defaultMotorSpeed);
            break;
        case 'x':
            increaseMotorSpeed();
            break;
        case 'z':
            decreaseMotorSpeed();
            break;
        case ' ':
            halt();
            break;
        case 'e':
            halt();
            state = END;
            std::cout << "Director: Remote mode was exited!" << std::endl;
        }
        setMotorSpeed();
    }
}

void CustomerRobot::autoMode()
{
    switch (autoState)
    {
    case AUTO_IDLE:
        if (purchaseConfirmation && orderReady)
        {
            autoState = AUTO_MOVE_PICKUP_COUNTER;
            std::cout << "Customer " + std::to_string(robotID) + ": I am heading to pickup counter" << std::endl;
        }
        break;
    case AUTO_MOVE_ORDER_COUNTER:
        if (moveState == INTERMEDIATE_MOVE_FINISH)
        {
            moveState = INTERMEDIATE_MOVE_IDLE;
            autoState = AUTO_CUSTOMER_ORDER;
        }
        else
        {
            move(CUSTOMER_ORDER_COUNTER_X, CUSTOMER_ORDER_COUNTER_Z, - M_PI / 2);
        }
        break;
    case AUTO_MOVE_PICKUP_COUNTER:
        if (moveState == INTERMEDIATE_MOVE_FINISH)
        {
            moveState = INTERMEDIATE_MOVE_IDLE;
            autoState = AUTO_CUSTOMER_PICKUP;
        }
        else
        {
            move(CUSTOMER_PICKUP_COUNTER_X, CUSTOMER_PICKUP_COUNTER_Z, M_PI);
        }
        break;
    case AUTO_MOVE_STARTING_POSITION:
        if (moveState == INTERMEDIATE_MOVE_FINISH)
        {
            moveState = INTERMEDIATE_MOVE_IDLE;
            autoState = AUTO_IDLE;
            resetOrdering();
        }
        else
        {
            move(startXPos, startZPos, startHeading * (M_PI / 180));
        }
        break;
    case AUTO_CUSTOMER_ORDER:
        makeOrder();
        break;
    case AUTO_CUSTOMER_PAY:
        payOrder();
        break;
    case AUTO_CUSTOMER_PICKUP:
        pickupOrder();
        break;
    }
}

void CustomerRobot::processData()
{
    if (currentData.size() > 0)
    {
        if (std::isdigit(currentData[0]))
        { // State changes start with a digit
            state = std::stoi(currentData);
            currentData = "";
            return;
        }
        switch (currentData[0])
        {
        case '?': // Print balance
            printBalance();
            break;
        case '~': // End controller
            state = END;
            printBalance();
            break;
        case '-': // Does not exist on menu
            currentOrderItemExists = false;
            autoState = AUTO_CUSTOMER_PAY;
            break;
        case '+': // Exists on menu
            currentOrderItemExists = true;
            break;
        case '$': // Price return
            currentOrderPrice = std::stod(currentData.substr(1, currentData.size() - 1));
            autoState = AUTO_CUSTOMER_PAY;
            break;
        case '*': // Order ready be picked up
            orderReady = true;
            break;
        default:
            currentOrder = currentData;
            std::cout << "Customer " + std::to_string(robotID) + ": I am heading to order counter" << std::endl;
            autoState = AUTO_MOVE_ORDER_COUNTER;
            break;
        }
    }
}

void CustomerRobot::makeOrder()
{
    std::cout << "Customer " + std::to_string(robotID) + ": Hi Staff, I would like to order " + currentOrder << std::endl;
    std::string message{currentOrder + std::to_string(robotID)};
    sendMessage(message, 5);
    std::cout << "Customer " + std::to_string(robotID) + ": *waiting to pay*" << std::endl;
    autoState = AUTO_IDLE;
}

void CustomerRobot::payOrder()
{
    std::string message;
    if (currentOrderItemExists && currentOrderPrice <= mBalance)
    {
        std::cout << "Customer " + std::to_string(robotID) + ": *has enough money*" << std::endl;
        std::cout << "Customer " + std::to_string(robotID) + ": Hi Staff, I will buy it" << std::endl;
        std::cout << "Customer " + std::to_string(robotID) + ": *pays by card/cash*" << std::endl;
        message = ">";
        purchaseConfirmation = true;
        mBalance -= currentOrderPrice;
        autoState = AUTO_IDLE;
    }
    else
    {
        std::cout << "Customer " + std::to_string(robotID) + ": *doesn't have enough money or made a boo boo*" << std::endl;
        std::cout << "Customer " + std::to_string(robotID) + ": Oops, I will cancel the order" << std::endl;
        message = "<";
        autoState = AUTO_MOVE_STARTING_POSITION;
    }
    sendMessage(message, 5);
}

void CustomerRobot::pickupOrder()
{
    std::cout << "Customer " + std::to_string(robotID) + ": I got my " + currentOrder << std::endl;
    // Send message to staff that order is picked up
    sendMessage("*", 5);
    autoState = AUTO_MOVE_STARTING_POSITION;
    std::cout << "Customer " + std::to_string(robotID) + ": I am returning to starting point" << std::endl;
}

void CustomerRobot::resetOrdering()
{
    currentOrderItemExists = false;
    orderReady = false;
    currentOrder = "";
    currentOrderPrice = 0;
    purchaseConfirmation = false;
    sendMessage("Order Complete", 6);
}

CustomerRobot::~CustomerRobot() {}