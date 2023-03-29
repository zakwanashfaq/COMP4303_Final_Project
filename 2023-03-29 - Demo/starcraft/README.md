# How the bot works for the demo
Demo link: [https://www.youtube.com/watch?v=mezimxduh7g](https://www.youtube.com/watch?v=mezimxduh7g)

- **Build-orders partially implemented with units being built toward a specific army goal:**
Build order is partially implemented with units being build towards our proposed goal. For the demo, we are using a dequeue array inside Starterbot.cpp file for build order along with a bool array named “isBuild” of dequeue size with all values set as “false” initially. The dequeue has number of workers owned and the name of unit to be build. For example, {9, “pylon”} means build pylon when we reach 9 workers. We loop for each item in dequeue with isBuild value of false, and once it is built, we set the status of that item to true in “isBuild” bool array. For the final submission, we will be using a text file from bin folder to get build order.

- **Enemy base is scouted at the beginning of the match and successfully found**: 
To do this we are looking for all unexplored places in the map with tile level precision and also at 15 tiles intervals to make the scouting more efficient. We also have a "tried to visit" grid that holds integer value of how many time the scout tried to visit that location. If that value exceeds 500 the scout will not visit that location anymore, as that location is not reachable by the scout.The scout is looking for enemy building units and once the enemy building unit is found it will save the location of that to be the base. This will be further improved to detect the enemy main building.

- **An attack takes place on the enemy base, ideally doing some damage or winning a game:** 
Once the enemy base is found and we have a certain number of units for an attack, all the units will march to the enemy base to do an attack. If we are able to detect the enemy early on there is a higher probability of winning, because they did not have enough defense built. 

- **Whatever your chosen area of specialization was should be partially implemented:** For this demo we have implemented enemy detection at our base and attacking the enemy if they are at our base. This is a part of our specialization where we fend off early attacks to survive until mid game and launch an attack with stronger units. For the initial demo, we are detecting if there are enemy units within "600 position units" within our main base depot. If there are enemy units present all our units stop whatever they are doing and attack the enemy unit to protect our base and minimize damage. 
