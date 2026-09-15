# Middle

**Middle is a game engine developed to run an educational Algebra Rewrite System.**

The Algebra Rewrite System is designed to let users perform algebraic manipulations interactively. Algebraic objects are currently called **Bubbles**, a name that comes from an earlier visual design where the objects were planned to be bubble-shaped. The visual design has since changed toward rectangular shapes, so this terminology will likely change in the future.

## Algebra Rewrite System

The application represents algebraic expressions as structured objects that can be manipulated directly. The goal is to make algebraic transformations explicit and interactive rather than treating an expression as plain text.

### Expansion and compression

Expressions can be **expanded** according to their algebraic structure.

For example:

```text
3 × 3  →  3 + 3 + 3

3³  →  3 × 3 × 3
```

The reverse operation, **compression**, can turn repeated structures back into multiplication or powers:

```text
3 + 3 + 3  →  3 × 3

3 × 3 × 3  →  3³
```

Compression can also be used to extract common factors. For example:

```text
3 + 3 + 3  →  3(1 + 1 + 1)
```

These operations work with arbitrary algebraic structures, not just individual numbers. For example, a compound expression can itself be used as a factor:

```text
(3x + 2y)(4 + 1)

→ 4(3x + 2y) + (3x + 2y)
```

### Regrouping

Terms under an addition structure can be freely regrouped.

For example:

```text
2 - 3 + 3 × 2

→ (2 - 3) + (1 + 1 + 1) × 2
```

This allows the user to explicitly restructure an expression while preserving its meaning.

### Inventory / algebraic objects

Algebraic objects can be copied into an **inventory/deck** and later inserted back into an expression.

Inventory items can be inserted in several ways:

1. Add the item to both sides of an equation.
2. Multiply both sides of an equation by the item.
3. Raise both sides of an equation to the item's power.
4. Multiply a term by an invariant such as `x/x`.
5. Add an invariant such as `x - x` to a term.

Inventory items can also be freely **negated** or **inverted**.

This allows algebraic transformations to be performed through explicit operations rather than by directly editing the expression.

### Functions

The system supports function objects that can be manipulated algebraically.

Functions can also be substituted with their definition formulas, with the corresponding input variables substituted into the definition.

### Summations

The system supports summation objects.

A summation can be sequentially expanded until its index reaches its upper limit. Algebraic transformations can also be applied to summations.

For example, compression can be used to extract a common factor from an entire summation.

### Definitions and substitution

Objects can be defined as equal to other values or expressions and subsequently substituted wherever the corresponding object occurs.

For example:

```text
x = 3

y = x - 7

→ y = 3 - 7
```

Definitions can contain arbitrary algebraic structures. For example:

```text
2 + 3e² = 3 - z
```

could be used to substitute `3 - z` wherever that structure occurs in another expression.

### Procedures

The application also supports **procedures**, which are sequences of algebraic manipulations.

This can be used to encode common algebraic identities or other multi-step transformations. Instead of manually performing every step, a user can apply a procedure that performs the sequence automatically.

## Planned algebraic operations

Logarithms and division are not currently supported, but are planned for the future.

One of the design constraints of the current system is that algebraic operations are implemented using a relatively small set of primitive structural operations:

- copy
- delete
- replace
- reparent

This is intentional. It makes the transformations easier to reason about and allows the underlying operations to remain explicit for educational purposes.

However, division and logarithms introduce operations that are more naturally implemented using additional iterative methods. I have not yet found an approach that makes these operations feel as intuitive and consistent with the existing primitive operations, so they remain future work.

---

# Middle Engine

Middle is a **sparse-set Entity Component System (ECS)** designed to support the Algebra Rewrite System.

Entities are identified separately from their components. Components are stored in dense array pools, while entities maintain references to their corresponding component locations.

### Component caches

Gameplay systems can define **CompCaches** that cache the entities matching a particular set of components.

For example, a system interested in entities containing:

```text
MouseIntersectable
Button
Rectangle
```

can maintain a vector of relevant entity IDs.

The component indices for those entities are stored at corresponding positions in the component arrays. If entity `5` is at index `3` in the relevant-ID vector, its component can be accessed through index `3` in the corresponding component arrays.

This allows systems to iterate over relevant entities without repeatedly searching the entire entity set.

The sparse-set architecture was chosen because components are expected to move frequently during runtime, while dense component storage provides efficient iteration over the actual component data.

There is currently significant room for improvement in memory usage; the current implementation uses substantially more memory than the final system should require.

---

# Scene System

Middle uses custom scene files with the `.midsc` extension. The scene format is still in an early stage of development.

Scenes can be imported into other scenes. Imported entities appear in the importing scene as **Ghost IDs** within the entity ID space.

Ghost IDs are not serialized individually. Instead, the serializer only stores the top-level parent of an imported scene, which contains the path to the source scene file.

This allows imported scenes to remain references to their original scene files rather than duplicating their contents during serialization.

---

# Visual Editor

Middle includes a simple visual editor primarily intended for debugging, inspecting entities, and creating basic scene content.

The editor currently supports:

- Creating basic entities
- Adding components to entities
- Importing components
- Inspecting component values
- Editing component values
- Importing scenes
- Adding systems to scenes
- Activating systems through scene configuration

Component editing is integrated with the component definitions, allowing the editor to expose common data types through **Dear ImGui**.

The current system activation workflow is still experimental. Systems are currently activated by placing them physically into a scene through the editor. This is an area planned for redesign.

---

# Project Status

Middle and the Algebra Rewrite System are active development projects.

The engine architecture, scene format, editor, and algebra system are all evolving as the requirements of the application become clearer.

The main focus of the project is currently on making algebraic transformations:

- structurally explicit
- easy to understand
- composable
- interactive
- suitable for educational use

while keeping the underlying engine architecture simple enough to reason about and extend.