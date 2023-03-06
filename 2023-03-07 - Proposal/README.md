# Project Title: Starcraft Bot

  

**Group members**:

- Zakwan Ashfaq Zian | 201950250

- Deep Vasantbhai Choudhary | 201855905

  

**Specialty**: Protoss Race

  

----------

  

  

**Overview:** For the project, we will be going forward with implementing an agent for the StarCraft project with Protoss as the selected race. The goal of the project is to focus on an interesting AI speciality and implement it in a real-time game environment along with other requirements. We plan to build a bot that will defend early on, to survive rush attacks and attack mid-game to maximize win rate.

  

  

**Chosen Specialty:** Our Chosen specialty will be to make defensive units and structures; and place them in strategic positions using a cellular automata algorithm, to block early rush attacks that might wipe us out. Our algorithm will be smart enough to lay out a pattern of structures based on our spawn location, to maximize defense ability. One example of this will be making sure the cannons overlap each other, so, if one is being attacked, other cannons can assist in defending it. Another example would be the cellular automata will recognize the choke points for our base, and put strategic defense structures near there to slow down the enemy. Effective pylon and canon placement will be the core focus of our cellular automata.

  

  

**Build order**: A modified Protoss 10/12 Gate Zeal build with preference to building cannons(subject to change). This build order will be read from a text file and the bot will automatically build out the structure and units.

8/9 – Pylon (scout if enemy went random)

10/17 – Gateway (scout if you have not already)

12/17 – Gateway (11/17 if you scouted late)

13/17 – Zealot, Pylon (in that order, hotkey and rally gate to choke)

17/25 – 2 Zealots, Pylon1 (in that order, hotkey and rally other gate to choke)

20/25 - Cybernetics Core

21/25 - Zealot

23/25 - Pylon

25/33 - Dragoon

27/33 - Citadel of Adun

29/33 - Gateway

29/33 - Zealot

31/33 - Pylon, Legs upgrade and Two Zealots

37/41 - Pylon

37/49 - Templar Archives

37/49 - Two Zealots, Two High Templars or Dark Templars

  

  

**Scouting**: Once we are at our 10 unit mark, we are going to use a probe to scout the map. If the probe seems difficult to produce a dragoon might be used early in the game to do some nearby sightings. We are going to have a list of every structure discovered and depending on how near it is to our base and what it's doing, we are going to revisit that location for an updated state.

  

  

**Defense**: Our main defense will mostly consist of cannons. They will automatically target enemy units and destroy them. Other moving units like Zealots will be used as extra muscle or backup as they are cheap and available early in the game.

  

  

**Attack Timing**: Our attacks will start during the mid-game period after defense is built and we have an excess of units that we can use to take enemy bases down. And from that point onwards, we will send an attack after every time a certain number of excess units are built.

  

  

**Expanding**: As we start to have enough units and structures for defense, we will start to look for new sources of resources. We will start by setting up a base at the next nearest mineral extraction site. As the game continues and our bot gathers more resources, we will start to build defensive structures around these sites to protect them from enemy attacks.