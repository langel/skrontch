# Skrontch Architecture

## System Overview

Skrontch is built around a hierarchical GUI system that provides flexible workspace management through windows, tabs, and split panes.

## Core Concepts

### Windows
- Top-level containers in the application
- Can be moved, resized, minimized, maximized, and closed
- Each window can contain one or more tab containers
- Windows maintain their own position and size state
- Windows can be docked or floating (future enhancement)

### Tabs
- Tab containers exist within windows
- Allow multiple views/tools to be open in the same window
- Users can switch between tabs, rearrange them, and close them
- Each tab can contain a split pane or direct content
- Tab bar shows active tab and allows switching

### Split Panes
- Resizable dividers that split a container (window or tab) into multiple regions
- Can split horizontally or vertically
- Can be nested to create complex layouts
- Each pane can contain another split pane, a tab container, or direct content
- Users can resize panes by dragging the divider
- Panes can be collapsed/expanded

## Component Hierarchy

```
Application
└── Window Manager
    └── Windows[]
        └── Tab Container
            └── Tabs[]
                └── Split Pane (optional)
                    ├── Pane A
                    │   └── Content/Tool
                    └── Pane B
                        └── Content/Tool
```

## Data Structures

### Window
- Position (x, y)
- Size (width, height)
- Title
- State (normal, minimized, maximized)
- Tab container reference
- Event handlers (close, resize, move)

### Tab Container
- List of tabs
- Active tab index
- Parent window reference
- Layout constraints

### Tab
- Title/label
- Content reference (tool instance)
- Icon (optional)
- State (active, dirty, etc.)

### Split Pane
- Orientation (horizontal/vertical)
- Split ratio/position
- Child panes (left/right or top/bottom)
- Minimum pane sizes
- Resize handle area

## Event Flow

1. SDL events captured by main event loop
2. Events routed to appropriate window based on mouse position
3. Window routes to active tab
4. Tab routes to content or split pane
5. Split pane routes to appropriate child pane
6. Content/tool handles the event

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

## State Management

- Window layouts and positions saved/restored
- Tab states preserved when switching
- Split pane ratios remembered
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
