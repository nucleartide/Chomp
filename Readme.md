# Chomp

boop

An action maze chase video game about navigating a maze, eating dots, and avoiding ghosts. Made in Unreal Engine 5.2; loosely inspired by Pacman.

## 🎮 Play the game

Please see the [latest releases on Google Drive](https://drive.google.com/drive/folders/1OTklrCTUU66ITB8hdL5YUrdoOs1tV6-E?usp=sharing) (not GitHub Releases) for a download link.

## 👀 Review the development process

In addition to the game itself, the Chomp project also intends to demonstrate how a game made in Unreal Engine 5 comes together, and the tradeoffs that were made under particular time and resource constraints.

A post-mortem technical write-up will be published when the project is done. Until then, here is a list of pull requests (categorized by gameplay system) that document the development process:

| Gameplay System | Pull Request |
| --- | --- |
| https://github.com/nucleartide/Chomp/labels/game%20feel | https://github.com/nucleartide/Chomp/pull/33 Add camera animations + camera shakes
| https://github.com/nucleartide/Chomp/labels/game%20feel | https://github.com/nucleartide/Chomp/pull/32 Add Pacman's chomping animation
| https://github.com/nucleartide/Chomp/labels/game%20feel | https://github.com/nucleartide/Chomp/pull/31 Add first pass at final game's look and feel
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/30 Rehaul game over screens with additional info
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/29 Implement a pause menu; refactor UI into componentized Widget Blueprints
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/28 Implement an options/settings menu
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/27 Finalize start menu UI/UX design
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/24 Fix scalability, resolution, aspect ratio, and fullscreen mode in shipping build
| https://github.com/nucleartide/Chomp/labels/consumables https://github.com/nucleartide/Chomp/labels/scoring | https://github.com/nucleartide/Chomp/pull/22 Implement bonus fruit
| https://github.com/nucleartide/Chomp/labels/health | https://github.com/nucleartide/Chomp/pull/21 Implement lives
| https://github.com/nucleartide/Chomp/labels/AI https://github.com/nucleartide/Chomp/labels/grid-aligned%20movement | https://github.com/nucleartide/Chomp/pull/20 Enable ghost wraparound
| https://github.com/nucleartide/Chomp/labels/scoring | https://github.com/nucleartide/Chomp/pull/18 Update scoring upon consuming energizer dots and ghosts
| https://github.com/nucleartide/Chomp/labels/consumables | https://github.com/nucleartide/Chomp/pull/17 Add ability to consume ghosts when game is in frightened state
| https://github.com/nucleartide/Chomp/labels/consumables https://github.com/nucleartide/Chomp/labels/game%20loop | https://github.com/nucleartide/Chomp/pull/16 Add energizers and frightened state
| https://github.com/nucleartide/Chomp/labels/AI | https://github.com/nucleartide/Chomp/pull/15 Implement AI behavior for Pinky and Clyde
| https://github.com/nucleartide/Chomp/labels/AI | https://github.com/nucleartide/Chomp/pull/14 Implement custom AI behavior for Inky cyan ghost
| https://github.com/nucleartide/Chomp/labels/AI | https://github.com/nucleartide/Chomp/pull/13 Implement timer-since-last-dot to force ghosts to leave house
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/12 Add game menus, among other things
| https://github.com/nucleartide/Chomp/labels/AI https://github.com/nucleartide/Chomp/labels/grid-aligned%20movement | https://github.com/nucleartide/Chomp/pull/11 Add remaining ghosts & fix bugs in grid aligned movement
| https://github.com/nucleartide/Chomp/labels/AI https://github.com/nucleartide/Chomp/labels/grid-aligned%20movement | https://github.com/nucleartide/Chomp/pull/10 Re-implement AI controller using new grid-aligned movement algorithm
| https://github.com/nucleartide/Chomp/labels/grid-aligned%20movement | https://github.com/nucleartide/Chomp/pull/9 Re-implement player movement to be grid-aligned
| https://github.com/nucleartide/Chomp/labels/bug | https://github.com/nucleartide/Chomp/pull/8 Reset ghost upon restart, among other things
| https://github.com/nucleartide/Chomp/labels/UI | https://github.com/nucleartide/Chomp/pull/6 Fix game over screens, among other things
| https://github.com/nucleartide/Chomp/labels/AI | https://github.com/nucleartide/Chomp/pull/5 Implement `.Pathfind` behavior for ghosts
| https://github.com/nucleartide/Chomp/labels/AI https://github.com/nucleartide/Chomp/labels/game%20loop | https://github.com/nucleartide/Chomp/pull/4 Add ghosts + lose condition
| https://github.com/nucleartide/Chomp/labels/pipeline | https://github.com/nucleartide/Chomp/pull/3 Add PR template
| https://github.com/nucleartide/Chomp/labels/game%20loop | https://github.com/nucleartide/Chomp/pull/2 Get game loop win condition only fully functional
