# Webots Cafeteria Simulation

This is a simulation of a cafeteria involving a staff and four customer robots. This simulation models the interactions between staff and customers, specifically
- customer queues and serving customers at the counter
- availability of menu items when requested by customers
- staff & customer transactions and maintaining balances/accounts after purchases
- 'making' orders and dropping off orders at a pickup counter

Communication between staff and customer robots are simulated by unique dialogue channels. Robots process dialogue data by broadcasting and receive string data packets on their own dialogue channels. This simulation also features a control mode to manually control the speed and movement of the robots. 

