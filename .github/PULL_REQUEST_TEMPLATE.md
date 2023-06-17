<summary>
<strong>Player</strong>: When <code>X</code>, I want to <code>Y</code>, so that <code>Z</code>.
</summary>

### Notion

> Provide a link to the Notion block corresponding to this job story.
>
> (Sorry folks, these blocks are internal to my Notion workspace!)

> *(link goes here)*

### Deliberate Practice

> Write down one skill that you'd like to improve in this pull request.
>
> **Examples**: Knowledge of a package. A sub-skill, such as creating particle effects. Putting in more hours (effort). Working more efficiently. Time estimation.

> *I want to...*

### Development Checklist

> Run through the following steps to complete your work.

* [ ] **Product requirements and game design**. Flesh these out in the corresponding Notion block. Make sure to consider:
    * [ ] **Model**. What is the user story? (See above.)
    * [ ] **Action**. How does the player perform actions? What inputs are needed?
    * [ ] **Rules**. The underlying mechanics.
    * [ ] **Feedback**. The sensory feedback presented to the player.
* [ ] **First draft**. Implement a working first pass at the problem. Try to think in modules: modify prefabs instead of the entire scene.
* [ ] **Final draft**. Once done, perform a general review of `Files changed` and revise anything that needs to be cleaned up.

### Quality Assurance Checklist

> Ensure that your work adheres to the following principles.

* [ ] **Modular**. Are you touching the scene file? If so, can you limit your changes to prefab files instead?
* [ ] **Editable**. Has any configuration been moved into ScriptableObjects, so that modified values persist even after exiting Play mode?
* [ ] **Debuggable**. Do you have debug views for the prefabs/systems that you are creating?
* [ ] **Learnable**. Consider how the player will learn what you implement in this pull request. Are the affordances appropriately communicated with signifiers?

more:

- [ ] Every PR should have a bug-free build
- [ ] Ensure architecture/modules are decoupled
- [ ] Otherwise clean up comments/formatting/naming/etc
