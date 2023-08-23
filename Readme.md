# Chomp

An action maze chase video game about navigating a maze, eating dots, and avoiding ghosts. Made in Unreal Engine 5.2; loosely inspired by Pacman.

## 🎮 Play the game

Please see the [latest releases on Google Drive](https://drive.google.com/drive/folders/1OTklrCTUU66ITB8hdL5YUrdoOs1tV6-E?usp=sharing) (not GitHub Releases) for a download link.

## 👀 Review the development process

In addition to the game itself, the Chomp project also intends to demonstrate how a game comes together, and the tradeoffs that were made under particular time and resource constraints.

A post-mortem write-up will be published when the project is done. Until then, here is a list of pull requests (categorized by gameplay system) that document the development process:

| Gameplay System | Pull Request |
| --- | --- |
| Consumables | Implement bonus fruit
| Health | Implement lives
| AI, grid-aligned movement | Enable ghost wraparound
| Scoring | Update scoring upon consuming energizer dots and ghosts
| Consumables | Add ability to consume ghosts when game is in frightened state
| Consumables, state machine | Add energizers and frightened state
| AI | Implement AI behavior for Pinky and Clyde
| AI | Implement custom AI behavior for Inky cyan ghost |
| AI | Implement timer-since-last-dot to force ghosts to leave house
| UI | Add game menus, among other things
| AI, grid-aligned movement | Add remaining ghosts & fix bugs in grid aligned movement
| AI, grid-aligned movement | Re-implement AI controller using new grid-aligned movement algorithm
| Character-Controls-Camera, grid-aligned movement | Re-implement player movement to be grid-aligned
| Bugfix | Reset ghost upon restart, among other things
| UI | Fix game over screens, among other things
| AI | Implement .Pathfind | behavior for ghosts
| AI, game loop | Add ghosts + lose condition
| Pipeline | Add PR template
| Game loop | Get game loop win condition only | fully functional
