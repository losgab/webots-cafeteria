#include "z5363966BaseRobot.hpp"

class StaffRobot : public BaseRobot {
    public:
        /**
         * @brief Construct a new Staff Robot object
         * 
         */
        StaffRobot();

        void run() override;
        void remoteControl() override;
        void autoMode() override;

        void processData() override;
    
        /**
         * @brief Checks the received order if can be made or not
         * 
         */
        void checkOrder();

        /**
         * @brief Places the order if confirmed paid.
         * 
         */
        void placeOrder();

        /**
         * @brief Prepares the order, implements a timer that increments with each time step, changes state when timer is done
         * 
         */
        void prepareOrder();

        /**
         * @brief Updates account
         * 
         */
        void updateAccount();

        /**
         * @brief Serves the order and resets for new order
         * 
         */
        void serveOrder();

        virtual void resetOrdering() override;

        /**
         * @brief Destroy the Customer Robot object
         * 
         */
        ~StaffRobot();
    private:
        std::ofstream accountFile;

        int currentOrderWaitTime;
        int orderTimer;
        int currentCustomer;
        int orderCounter;
        
        // Staff Auto States
        static constexpr int AUTO_STAFF_CHECK_ORDER {11};
        static constexpr int AUTO_STAFF_PLACE_ORDER {12};
        static constexpr int AUTO_STAFF_ORDER_PREP {13};
        static constexpr int AUTO_STAFF_ORDER_READY {14};
};