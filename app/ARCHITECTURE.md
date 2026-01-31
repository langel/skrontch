# Skrontch Architecture

## System Overview

Skrontch is built around a hierarchical GUI system that provides flexible workspace management
through windows, tabs, and split panes. Runtime ownership is modeled as AppState -> WindowState
-> TabState -> SplitTree -> PaneNode.

## Core Concepts

### Windows
- Top-level containers in the application
- Can be moved, resized, minimized, maximized, and closed
- Each window can contain one or more tab containers
- Windows maintain their own position and size state
- Closing the last window exits the app
- Windows can be docked or floating (future enhancement)

### Tabs
- Tab containers exist within windows
- Allow multiple views/tools to be open in the same window
- Users can switch between tabs and drag-reorder them
- The tab bar includes a + button to add a new tab
- Each tab owns a split tree root and focused pane

### Split Panes
- Resizable dividers that split a container (window or tab) into multiple regions
- Can split horizontally or vertically
- Can be nested to create complex layouts
- Each pane can contain another split pane, a tab container, or direct content
- Users can resize panes by dragging the divider
- Panes can be collapsed/expanded
- Pane headers include a detach affordance to spawn a new window

## Component Hierarchy

```
Application (AppState)
└── WindowState[]
    └── TabState[]
        └── SplitTree
            ├── PaneNode
            └── PaneNode
```

## Data Structures

### WindowState
- Position (x, y)
- Size (width, height)
- Title
- State (normal, minimized, maximized)
- Tab list + active tab index
- Event handlers (close, resize, move)

### TabState
- Title/label
- Split tree root + node list
- Focused pane node

### Split Tree / Pane Nodes
- Orientation (horizontal/vertical)
- Split ratio/position
- Child panes (left/right or top/bottom)
- Minimum pane sizes
- Resize handle area

## Event Flow

1. SDL events captured by main event loop
2. Events routed to the matching WindowState via SDL window ID
3. WindowState routes to the active TabState
4. TabState routes to the split tree / pane
5. Content/tool handles the event

## Rendering Flow

1. Main render loop clears screen
2. Window manager renders all windows in Z-order
3. Each window renders its tab container
4. Active tab renders its content
5. Split panes render their dividers and child panes recursively
6. Present to screen

## Tool Integration

Tools are content that can be placed in tabs or split panes:
- Graphics editor
- Sound editor
- Sprite sheet manager
- Palette editor
- Audio sequencer
- etc.

Each tool:
- Implements a standard interface (render, handle_event, update)
- Manages its own state
- Can request window/tab operations (e.g., open new tab)

## State Management & Persistence

- Window layouts, sizes, and positions are saved/restored
- Tab order, active tab, and focused pane are persisted
- Split tree structure and ratios are persisted
- Workspace saved to `disk/workspace/workspace.json` under `SDL_GetBasePath()`
- Saves are debounced (about 150ms) and triggered on state changes
- Tool-specific state managed by tools themselves

## Future Enhancements

- Docking system for windows
- Workspace presets
- Plugin system for custom tools
- Undo/redo system
- Project file format

## Design Notes (User Requirements)

- Build a component system for tool rendering and interaction once window panes are stable
- Provide a library of common GUI elements and support custom elements
- Define a markup language for UI elements (name, behavior, location, appearance)
- GUI elements should specify which cursor appears in given scenarios
