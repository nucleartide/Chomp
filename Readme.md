# Chomp

An action maze chase video game about navigating a maze, eating dots, and avoiding ghosts. Made in Unreal Engine 5.2; loosely inspired by Pacman.

## 🎮 Play the game

Please see the [latest releases on Google Drive](https://drive.google.com/drive/folders/1OTklrCTUU66ITB8hdL5YUrdoOs1tV6-E?usp=sharing) (not GitHub Releases) for a download link.

## 👀 Review the development process

In addition to the game itself, the Chomp project also intends to demonstrate how a game comes together, and the tradeoffs that were made under particular time and resource constraints.

A post-mortem write-up will be published when the project is done. Until then, here is a list of pull requests (categorized by gameplay system) that document the development process:

| Gameplay System | Pull Request |
| --- | --- |
| consumables, scoring | https://github.com/nucleartide/Chomp/pull/22 Implement bonus fruit
| health | https://github.com/nucleartide/Chomp/pull/21 Implement lives
| AI, grid-aligned movement | https://github.com/nucleartide/Chomp/pull/20 Enable ghost wraparound
| scoring | https://github.com/nucleartide/Chomp/pull/18 Update scoring upon consuming energizer dots and ghosts
| consumables | https://github.com/nucleartide/Chomp/pull/17 Add ability to consume ghosts when game is in frightened state
| consumables, game loop | https://github.com/nucleartide/Chomp/pull/16 Add energizers and frightened state
| AI | https://github.com/nucleartide/Chomp/pull/15 Implement AI behavior for Pinky and Clyde
| AI | https://github.com/nucleartide/Chomp/pull/14 Implement custom AI behavior for Inky cyan ghost |
| AI | https://github.com/nucleartide/Chomp/pull/13 Implement timer-since-last-dot to force ghosts to leave house
| UI | https://github.com/nucleartide/Chomp/pull/12 Add game menus, among other things
| AI, grid-aligned movement | https://github.com/nucleartide/Chomp/pull/11 Add remaining ghosts & fix bugs in grid aligned movement
| AI, grid-aligned movement | https://github.com/nucleartide/Chomp/pull/10 Re-implement AI controller using new grid-aligned movement algorithm
| grid-aligned movement | https://github.com/nucleartide/Chomp/pull/9 Re-implement player movement to be grid-aligned
| bug | https://github.com/nucleartide/Chomp/pull/8 Reset ghost upon restart, among other things
| UI | https://github.com/nucleartide/Chomp/pull/6 Fix game over screens, among other things
| AI | https://github.com/nucleartide/Chomp/pull/5 Implement `.Pathfind` behavior for ghosts
| AI, game loop | https://github.com/nucleartide/Chomp/pull/4 Add ghosts + lose condition
| pipeline | https://github.com/nucleartide/Chomp/pull/3 Add PR template
| game loop | https://github.com/nucleartide/Chomp/pull/2 Get game loop win condition only fully functional

https://github.com/nucleartide/Chomp/labels/bug
