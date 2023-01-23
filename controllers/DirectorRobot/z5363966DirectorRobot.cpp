#include "z5363966DirectorRobot.hpp"

DirectorRobot::DirectorRobot()
	: robot(new webots::Robot()),
	  emitter(robot->getEmitter("emitter")),
	  receiver(robot->getReceiver("receiver")),
	  state(INITIAL),
	  allowedRemoteCommands({'1', '2', '3', '4', '5'}),
	  orderFile("../../Order.csv", std::ifstream::in),
	  currentOrder(""),
	  currentCustomer(0),
	  orderCounter(0)
{
	mKeyboard.enable(TIME_STEP);
	receiver->enable(TIME_STEP);
	receiver->setChannel(6);
}

void DirectorRobot::printCommandMenu()
{
	std::cout << "Director: This is a simulation for MTRN2500 Cafe." << std::endl;
	std::cout << "Director: Press [I] to reprint the commands." << std::endl;
	std::cout << "Director: Press [R] to remote control a robot." << std::endl;
	std::cout << "Director: Press [A] to enter the auto mode." << std::endl;
	std::cout << "Director: Press [Q] to quit all controllers." << std::endl;
}

void DirectorRobot::menuSelect(int key)
{
	switch (std::tolower(key))
	{
	case 'i':
		printCommandMenu();
		break;
	case 'r':
		std::cout << "Director: Please select the robot to control remotely:" << std::endl;
		std::cout << "Director: Press [1] to control the Purple Robot (Customer1)." << std::endl;
		std::cout << "Director: Press [2] to control the White Robot (Customer2)." << std::endl;
		std::cout << "Director: Press [3] to control the Gold Robot (Customer3)." << std::endl;
		std::cout << "Director: Press [4] to control the Green Robot (Customer4)." << std::endl;
		std::cout << "Director: Press [5] to control the Black Robot (Staff)." << std::endl;
		state = REMOTE_CONTROL_INITIALISE;
		break;
	case 'a':
		std::cout << "Director: Auto Mode starts" << std::endl;
		state = AUTO_INITIALISE;
		break;
	case 'q':
		state = END;
        emitter->setChannel(-1);
        emitter->send("~", 2);
		break;
	default:
		std::cout << "Director: Command not found." << std::endl;
		state = INITIAL;
		printCommandMenu();
	}
}

void DirectorRobot::startRemoteControl(int key)
{
	// Checks if command is valid
	char keyInput = static_cast<char>(key);
	if (std::count(allowedRemoteCommands.begin(), allowedRemoteCommands.end(), keyInput))
	{
		emitter->setChannel(std::stoi(std::string(1, keyInput)));
	}
	else
	{
		std::cout << "Director: Command not found." << std::endl;
		state = INITIAL;
		printCommandMenu();
		return;
	}
	state = REMOTE;
	std::cout << "Director: Robot " + std::string(1, keyInput) + " has now been told to be remotely controlled." << std::endl;
	emitter->send(std::to_string(state).data(), std::to_string(state).size() + 1);
}

void DirectorRobot::startAutoMode()
{
	state = AUTO;
	emitter->setChannel(-1);
	emitter->send("?", 2);
	emitter->send(std::to_string(state).data(), std::to_string(state).size() + 1);

	// Gets the first useless so next getline will be the actual data
	std::getline(orderFile, currentOrder);
}

void DirectorRobot::autoMode()
{
	if (state == AUTO)
	{
		if (std::getline(orderFile, lineInput))
		{
			std::stringstream lineStream{lineInput};
			std::string stringSegment;
			std::vector<std::string> orderLine;
			// menuLine will be in the form of {menuItem, prepTime, itemPrice}
			while (std::getline(lineStream, stringSegment, ','))
			{
				orderLine.push_back(stringSegment);
			}

			// Extract the information
			currentCustomer = std::stoi(orderLine[0]);
			currentOrder = orderLine[1];

			// Talk to Customer/Staff Robot
			emitter->setChannel(currentCustomer);
			emitter->send(currentOrder.data(), currentOrder.size() + 1);
			// std::cout << "Robot " + std::to_string(currentCustomer) + " ordered " + currentOrder << std::endl;
			state = AUTO_IDLE;
		}
        else
        {
            std::cout << "Director: All orders are completed" << std::endl;
            emitter->setChannel(-1);
            emitter->send("~", 2);
            state = END;
        }
	}
	else if (state == AUTO_IDLE)
	{
		if (receiver->getQueueLength() != 0) {
		    std::string data{(const char *)receiver->getData()};
		    receiver->nextPacket();
		    if (data.compare("Order Complete") == 0)
		    {
		        orderCounter++;
		        std::cout << "Director: Order " + std::to_string(orderCounter) + " complete" << std::endl;
		        state = AUTO;
		    }
		}
	}
}

void DirectorRobot::run()
{
	printCommandMenu();

	// Main Loop
	int key;
	while (robot->step(TIME_STEP) != -1)
	{
		key = mKeyboard.getKey();
		if (key != EOF)
		{
			switch (state)
			{
			case INITIAL:
				menuSelect(key);
				break;
			case REMOTE_CONTROL_INITIALISE:
				startRemoteControl(key);
				break;
			}
		}
		else if (state == REMOTE && receiver->getQueueLength() != 0)
		{
			std::string ret{(const char *)receiver->getData()};
			if (ret == "end")
			{
				state = END;
			}
			receiver->nextPacket();
		}

		switch (state)
		{
		case AUTO_INITIALISE:
			startAutoMode();
			break;
		case AUTO:
		case AUTO_IDLE:
			autoMode();
			break;
		case END:
			return;
		}
	}
}

DirectorRobot::~DirectorRobot() {}