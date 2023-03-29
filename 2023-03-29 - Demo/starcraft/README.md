# How the bot works for the demo
Demo link: `https://www.youtube.com/watch?v=mezimxduh7g`

- Build-orders partially implemented with units being built toward a specific army goal
- Enemy base is scouted at the beginning of the match and successfully found: To do this we are looking for all unexplored places in the map with tile level precision and also at 15 tiles intervals to make the scouting more efficient. We also have a "tried to visit" grid that holds integer value of how many time the scout tried to visit that location. If that value exceeds 500 the scout will not visit that location anymore, as that location is not reachable by the scout.The scout is looking for enemy building units and once the enemy building unit is found it will save the location of that to be the base. This will be further improved to detect the enemy main building.
- An attack takes place on the enemy base, ideally doing some damage or winning a game: Once the enemy base is found and we have a certain number of units for an attack, all the units will march to the enemy base to do an attack. If we are able to detect the enemy early on there is a higher probability of winning, because they did not have enough defence built. 
- Whatever your chosen area of specialization was should be partially implemented
- An explanation of how these things have been accomplished so far, and what features you have left to implement
- Link to the video in the root project README.md file



This folder contains the following 3 directories:

```
bin/ 
  - must contain your compiled StarterBot.exe
  - must contain any additional files you need in your bin dir
  - do not include any visualstudio output debugging files such as .pdb files
src/ 
  - must contain your bot source code
visualstudio/
  - must contain your visual studio project files
  - delete all temporary output vs directories, they contain massive files
    - visualstudio/Release
    - visualstudio/Debug
    - visualstudio/.vs
```

Simply copy your `bin`, `src`, and `visualstudio` directories from your STARTcraft project into this directory.

Notes: 
- This entire submission should be no more than a few megabytes at most. 
- Delete this folder if you are not doing the Starcraft project
