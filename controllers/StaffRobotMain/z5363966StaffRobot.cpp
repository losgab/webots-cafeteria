#include "z5363966StaffRobot.hpp"

StaffRobot::StaffRobot()
    : BaseRobot(),
      currentOrderWaitTime(0),
      orderTimer(0),
      currentCustomer(0),
      orderCounter(0)
{
    assignBalance();

    // Writes initial content to the account
    accountFile.open("../../Account.csv", std::ios::out | std::ios::trunc);
    accountFile << "Order,Item,Customer,Account Balance ($)" << std::endl;
    accountFile << "0,,," << std::setprecision(2) << std::fixed << mBalance << std::endl;
}

void StaffRobot::run()
{
    // Initial mode: waits for what mode to enter into

    while (step(TIME_STEP) != -1)
    {
        this->currentKey = mKeyboard.getKey();

        currentData = receiveMessage();
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

void StaffRobot::remoteControl()
{
    if (currentKey != EOF)
    {
        setMotorPosition();
        switch (std::tolower(currentKey))
        {
        case webots::Keyboard::UP:
            moveForward(defaultMotorSpeed);
            break;
        case webots::Keyboard::LEFT:
            turnLeft(defaultMotorSpeed);
            break;
        case webots::Keyboard::DOWN:
            moveBackward(defaultMotorSpeed);
            break;
        case webots::Keyboard::RIGHT:
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

void StaffRobot::autoMode()
{
    switch (autoState)
    {
    case AUTO_IDLE:
        break;
    case AUTO_MOVE_ORDER_COUNTER:
        std::cout << "Staff: I am heading to order counter" << std::endl;
        // move(STAFF_ORDER_COUNTER_X, STAFF_ORDER_COUNTER_Z);
        autoState = AUTO_STAFF_CHECK_ORDER;
        break;
    case AUTO_MOVE_PICKUP_COUNTER:
        // move to pickup counter
        // If arrived then change state to AUTO_STAFF_ORDER_READY
        autoState = AUTO_STAFF_ORDER_READY;
        break;
    case AUTO_MOVE_STARTING_POSITION:
        std::cout << "Staff: I am returning to starting point" << std::endl;
        // move
        if (purchaseConfirmation)
        {
            autoState = AUTO_STAFF_ORDER_PREP;
        }
        else
        {
            // if arrived back there then back to idle
            autoState = AUTO_IDLE;
        }
        break;
    case AUTO_STAFF_CHECK_ORDER:
        checkOrder();
        break;
    case AUTO_STAFF_PLACE_ORDER:
        placeOrder();
        break;
    case AUTO_STAFF_ORDER_PREP:
        prepareOrder();
        break;
    case AUTO_STAFF_ORDER_READY:
        // move to pick up tile
        serveOrder();
        break;
    }
}

void StaffRobot::processData()
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
        case '~': // Controller is told to quit
            state = END;
            accountFile.close();
            printBalance();
            break;
        case '<': // Purchase fail
            purchaseConfirmation = false;
            resetOrdering();
            autoState = AUTO_MOVE_STARTING_POSITION;
            break;
        case '>': // Purchase success
            purchaseConfirmation = true;
            autoState = AUTO_STAFF_PLACE_ORDER;
            break;
        case '*': // Item picked up from counter
            autoState = AUTO_MOVE_STARTING_POSITION;
            break;
        default:
            currentOrder = currentData;
            currentCustomer = currentOrder.back() - '0';
            currentOrder = currentOrder.substr(0, currentData.size() - 1);
            autoState = AUTO_MOVE_ORDER_COUNTER;
            break;
        }
    }
}

void StaffRobot::checkOrder()
{
    std::string lineInput;
    std::string menuItem, prepTime, itemPrice;
    std::cout << "Staff: *checking if item exists on menu*" << std::endl;

    std::ifstream menuFile{"../../Menu.csv", std::ifstream::in};

    // Check whether order exists in the menu
    while (std::getline(menuFile, lineInput))
    {
        std::stringstream lineStream{lineInput};
        std::string stringSegment;
        std::vector<std::string> menuLine;

        // menuLine will be in the form of {menuItem, prepTime, itemPrice}
        // Extracts information into a vector
        while (std::getline(lineStream, stringSegment, ','))
        {
            menuLine.push_back(stringSegment);
        }

        if (currentOrder == menuLine[0])
        {
            std::cout << "Staff: *finds item on menu*" << std::endl;
            sendMessage("+", currentCustomer);
            std::cout << "Staff: Hi Customer " + std::to_string(currentCustomer) + ", the price for " + currentOrder + " is " + menuLine[2] + " dollars" << std::endl;
            currentOrderWaitTime = std::stoi(menuLine[1]) * 1000;
            currentOrderPrice = std::stod(menuLine[2]);
            std::string message{"$" + menuLine[2]};
            sendMessage(message, currentCustomer);
            autoState = AUTO_IDLE;
            return;
        }
    }
    std::cout << "Staff: Hi Customer " + std::to_string(currentCustomer) + ", oh no, we don't have " + currentOrder + " in our menu" << std::endl;
    sendMessage("-", currentCustomer);
}

void StaffRobot::placeOrder()
{
    if (purchaseConfirmation)
    {
        updateAccount();
        std::cout << "Staff : Thanks for your order. It will be ready in " + std::to_string(currentOrderWaitTime / 1000) + " seconds" << std::endl;
        std::cout << "Staff: *places order, adds into account, prepares order*" << std::endl;
    }
    autoState = AUTO_MOVE_STARTING_POSITION;
}

void StaffRobot::updateAccount()
{
    orderCounter++;
    mBalance += currentOrderPrice;
    accountFile << std::to_string(orderCounter) + "," + currentOrder + "," + std::to_string(currentCustomer) + "," << std::setprecision(2) << std::fixed << mBalance << std::endl;
}

void StaffRobot::prepareOrder()
{
    orderTimer += TIME_STEP;
    if (orderTimer >= currentOrderWaitTime)
    {
        std::cout << "Staff: *order is prepared, moving to the pickup counter*" << std::endl;
        autoState = AUTO_MOVE_PICKUP_COUNTER;
    }
}

void StaffRobot::serveOrder()
{
    std::cout << "Staff: Hi customer " + std::to_string(currentCustomer) + ", your " + currentOrder + " is ready, please proceed to pickup counter" << std::endl;
    // Inform customer that order is ready to be picked up
    sendMessage("*", currentCustomer);
    resetOrdering();
    autoState = AUTO_IDLE;
}

void StaffRobot::resetOrdering()
{
    purchaseConfirmation = false;
    currentOrderWaitTime = 0;
    orderTimer = 0;
    currentCustomer = 0;
    currentOrder = "";
    currentOrderPrice = 0;
    autoState = AUTO_IDLE;
}

StaffRobot::~StaffRobot() {}