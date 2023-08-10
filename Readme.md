# Chomp

boop

> *A game about chomping on foods and avoiding ghosts, that demonstrates and speaks to the end-to-end process of shipping a game in Unreal Engine. Made in UE 5.2.*

https://github.com/nucleartide/Chomp/assets/914228/bbcde674-6839-4668-b1c2-bd29c907de08

## 🎮 Play the game

Please see the [latest releases](https://github.com/nucleartide/Chomp/releases) for a download link.

## Rationale

*Chomp* is an Unreal Engine–adaptation of the game mechanics that you see in *Pacman*.

As such, you can use this project as a reference point for creating games in Unreal. Whether:

- you're looking for examples of coding best practices in Unreal, or
- you are a Unity dev curious about how to construct Unreal projects, or
- you are simply looking for code examples that are more complex than usual coding tutorials,

This project will have something for you.

Take a look at the list of topics below to study the coding problems that are relevant to your own games:

### Tutorial hell

...

### C++ versus Blueprints

...

### UI management

...

### Level editing and level generation

...

### A* implementation

...

### AI and Behavior Tree (BT) implementation

Behavior Trees are a way to visually compose priority-based behavior. Their main benefit is in their visual construction, which makes it easy to delegate behavior tree authoring from programmers to designers.

You can hear about more benefits from ChatGPT: `what are the benefits of behavior trees versus a hard-coded AI implementation`

While I created a dummy behavior tree that runs in a Blueprint subclass of `AGhostAIController`, I rely on a hard-coded implementation for my game's AI for the sake of time constraints. With more time, I might explore how refactoring my AI implementation into a behavior tree would benefit AI exploration and iteration.

### Chasing the core game loop

...

### Agile practices

...

### Question-Driven Development with ChatGPT

...

### Event-driven gameplay architecture

...

### Productivity

...

### Debugging with Rider

...

### Test-Driven Development (TDD) with Unreal's Automation Framework

TODO: Discuss the following:

* being clear about what state is mutated (function names) or read (args)
* isolating integration with engine features that return unexpected results (`GetWorld()`, collision checks)

### Garbage collection with Unreal's `UOBJECT` system

...

### Benefits of functional programming

TODO: Discuss the implementation of the techniques in:

* http://sevangelatos.com/john-carmack-on/

### Game juice and polish

...

### Project timelines, the 5 phases of game development

* Ideation, pre-prod, prod, post-prod, release, feedback & playtesting

### More topics

* Iteration loop for productivity
* Using a Trello / Kanban board

## Contributing

TODO.
