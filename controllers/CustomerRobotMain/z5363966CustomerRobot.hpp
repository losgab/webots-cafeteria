#include "z5363966BaseRobot.hpp"

class CustomerRobot : public BaseRobot {
    public:
        /**
         * @brief Construct a new Customer Robot object
         * 
         */
        CustomerRobot();

        virtual void run() override;
        virtual void remoteControl() override;
        virtual void autoMode() override;

        void processData() override;    

        /**
         * @brief Sends order to the Staff
         * 
         */
        void makeOrder();

        /**
         * @brief Pays for order if able to buy the item ordered, else returns to starting position
         * 
         */
        void payOrder();

        /**
         * @brief Pick up order from pickup counter
         * 
         */
        void pickupOrder();

        virtual void resetOrdering() override;

        /**
         * @brief Destroy the Customer Robot object
         * 
         */
        ~CustomerRobot();
    private:
        bool currentOrderItemExists;
        bool orderReady;
        
        // Customer Auto States
        static constexpr int AUTO_CUSTOMER_ORDER {11};
        static constexpr int AUTO_CUSTOMER_PAY {12};
        static constexpr int AUTO_CUSTOMER_PICKUP {13};
        
        // Positions
        static constexpr double CUSTOMER_ORDER_QUEUE_X {0};
        static constexpr double CUSTOMER_ORDER_QUEUE_Z {0.125};
        static constexpr double CUSTOMER_ORDER_COUNTER_X {0.375};
        static constexpr double CUSTOMER_ORDER_COUNTER_Z {0.375};
        static constexpr double CUSTOMER_PICKUP_COUNTER_X {0.375};
        static constexpr double CUSTOMER_PICKUP_COUNTER_Z {-0.375};
};
