# Pac-T-Man
Project to implement my own **Pac-Man** game using *DirectX 11*. As an inspiration, the original version from 1980 was used.

<p align="center">  
  <img src="Doc/intro.gif">
</p>

There is dependency to *DirectXTK* which could be easily avoided but it is trivial to satisify thru *NuGet package* thus I decided to keep it.

# Controls
- `Left/Right/Up/Down arrow` - movement.

<p align="center">  
  <img src="Doc/game.gif">
</p>

The feeling from the game should be similar as you would have from playing the 1980's version. Just a speed is litle bit higher.

# Features
- You can move around.
- You can be eaten by the ghosts.
- You can eat energizer (the big dot) and hunt the ghosts.
- The game has different stages (ghosts can be chasing **Pac-Man** or running away).
- Each ghost has its unique AI based on the 1980's version. Only one difference here - there is [known and documented bug](http://donhodges.com/pacman_pinky_explanation.htm) in Pinky and Inky behavior. I did not implmenet this to my version :blush:

# Additional information
Sprite sheet taken from [here](https://www.nicepng.com/ourpic/u2q8q8w7i1e6t4e6_pacman10-hp-sprite-pacman-sprite-sheet-png/). To compile the project, you need to have [DirectXTK](https://github.com/microsoft/DirectXTK) which really simplifies *DirectX* usage. Solution is for *Visual Studio 2017*.

World is generated from a 2D map in a form of separated cubes. Simple optimization to connect adjanced cubes is done to minimaze number of the triangles and eliminate problems such as z-fighting. You can see it here from the bottom view:

<p align="center">  
  <img src="Doc/map.png">
</p>

Result is fully 3D world with dynamic point light inhabited by 2D billboards. Speed is locked to 60 PFS.

If you are interested in more details about **Pac-Man** in general, I can recommned these two sites what I was using as a reference and guide:

- [Understanding Pac-Man Ghost Behavior](https://gameinternals.com/understanding-pac-man-ghost-behavior)
- [The Pac-Man Dossier](https://www.gamasutra.com/view/feature/132330/the_pacman_dossier.php)

# Screenshots
<p align="center">  
  <img src="Doc/sc2.png">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; <img src="Doc/sc1.png">
</p>

