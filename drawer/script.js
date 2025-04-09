document.addEventListener('DOMContentLoaded', () => {
    const canvas = document.getElementById('drawingCanvas');
    const grid_canvas = document.getElementById('gridCanvas');
    const ctx = canvas.getContext('2d');
    const grid_ctx = grid_canvas.getContext('2d');
    
    // Set actual canvas dimensions to base size (256x240)
    canvas.width = 256;
    canvas.height = 240;
    grid_canvas.width = 768;
    grid_canvas.height = 720;

    const palette_container = document.getElementById('nesPalette');
    const clear_button = document.getElementById('clearCanvas');
    const save_button = document.getElementById('saveImage');
    const brush_size_input = document.getElementById('brushSize');
    const brush_size_value = document.getElementById('brushSizeValue');
    const toggle_grid_button = document.getElementById('toggleGrid');
    const reduce_colors_button = document.getElementById('reduceColors');
    
    // Tool buttons
    const pencil_tool = document.getElementById('pencilTool');
    const brush_tool = document.getElementById('brushTool');
    const bucket_tool = document.getElementById('bucketTool');
    const marquee_tool = document.getElementById('marqueeTool');
    
    // Clipboard buttons
    const copy_button = document.getElementById('copySelection');
    const paste_button = document.getElementById('pasteSelection');
    const cut_button = document.getElementById('cutSelection');
    
    let is_drawing = false;
    let selected_color = '#000000';
    let current_tool = 'pencil';
    let brush_size = 1;
    let selection = null;
    let clipboard = null;
    let last_x, last_y;
    let show_grid = false;

    // Initialize canvas with NES color 0x0F background
    ctx.fillStyle = '#004058';  // NES color 0x0F
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw grid
    function draw_grid() {
        grid_ctx.clearRect(0, 0, grid_canvas.width, grid_canvas.height);
        if (!show_grid) return;

        grid_ctx.strokeStyle = '#000000';
        grid_ctx.lineWidth = 1;

        // Draw vertical lines (every 48 pixels)
        for (let x = 0; x <= grid_canvas.width; x += 48) {
            grid_ctx.beginPath();
            grid_ctx.moveTo(x, 0);
            grid_ctx.lineTo(x, grid_canvas.height);
            grid_ctx.stroke();
        }

        // Draw horizontal lines (every 48 pixels)
        for (let y = 0; y <= grid_canvas.height; y += 48) {
            grid_ctx.beginPath();
            grid_ctx.moveTo(0, y);
            grid_ctx.lineTo(grid_canvas.width, y);
            grid_ctx.stroke();
        }
    }

    // Toggle grid
    toggle_grid_button.addEventListener('click', () => {
        show_grid = !show_grid;
        draw_grid();
        // Save grid state to localStorage
        localStorage.setItem('showGrid', show_grid);
    });

    // Load grid state from localStorage
    const saved_grid_state = localStorage.getItem('showGrid');
    if (saved_grid_state !== null) {
        show_grid = saved_grid_state === 'true';
        draw_grid();
    }

    // Create color palette
    nes_palette.forEach(color => {
        const swatch = document.createElement('div');
        swatch.className = 'color-swatch';
        swatch.style.backgroundColor = color;
        swatch.dataset.color = color; // Store original color in data attribute
        swatch.addEventListener('mousedown', () => select_color(color, swatch));
        palette_container.appendChild(swatch);
    });

    // Retrieve last selected color from localStorage and apply selection
    const stored_color = localStorage.getItem('selectedColor');
    if (stored_color) {
        selected_color = stored_color;
        // Find the swatch using the data-color attribute
        const swatch = document.querySelector(`.color-swatch[data-color="${stored_color}"]`);
        if (swatch) {
            swatch.classList.add('selected');
        }
    }
    else {
        // Select first color by default if no stored color
        const first_swatch = document.querySelector('.color-swatch');
        if (first_swatch) {
            first_swatch.classList.add('selected');
            selected_color = first_swatch.dataset.color; // Get color from data attribute
            localStorage.setItem('selectedColor', selected_color);
        }
    }

    // Update localStorage when color changes
    function select_color(color, swatch) {
        document.querySelectorAll('.color-swatch').forEach(s => s.classList.remove('selected'));
        swatch.classList.add('selected');
        selected_color = color;
        localStorage.setItem('selectedColor', color);
    }

    // Retrieve last selected tool and brush size from localStorage
    const stored_tool = localStorage.getItem('selectedTool');
    const stored_brush_size = localStorage.getItem('brushSize');

    if (stored_tool) {
        set_active_tool(stored_tool);
    }

    if (stored_brush_size) {
        brush_size = parseInt(stored_brush_size);
        brush_size_input.value = brush_size;
        brush_size_value.textContent = brush_size;
    }

    // Tool selection
    function set_active_tool(tool) {
        current_tool = tool;
        document.querySelectorAll('.tool-button').forEach(btn => btn.classList.remove('active'));
        document.getElementById(`${tool}Tool`).classList.add('active');
        localStorage.setItem('selectedTool', tool);
    }

    pencil_tool.addEventListener('click', () => set_active_tool('pencil'));
    brush_tool.addEventListener('click', () => set_active_tool('brush'));
    bucket_tool.addEventListener('click', () => set_active_tool('bucket'));
    marquee_tool.addEventListener('click', () => set_active_tool('marquee'));

    // Brush size control
    brush_size_input.addEventListener('input', (e) => {
        brush_size = parseInt(e.target.value);
        brush_size_value.textContent = brush_size;
        localStorage.setItem('brushSize', brush_size);
    });

    // Drawing functionality
    canvas.addEventListener('mousedown', start_drawing);
    canvas.addEventListener('mousemove', draw);
    canvas.addEventListener('mouseup', stop_drawing);
    canvas.addEventListener('mouseout', handle_mouse_out);
    canvas.addEventListener('mouseenter', (e) => {
        // Check if any mouse button is pressed (buttons property is a bitmask)
        if (e.buttons === 0) {
            is_drawing = false;
        }
    });

    function get_canvas_coordinates(e) {
        const rect = canvas.getBoundingClientRect();
        const scale_x = canvas.width / rect.width;
        const scale_y = canvas.height / rect.height;
        return {
            x: Math.floor((e.clientX - rect.left) * scale_x),
            y: Math.floor((e.clientY - rect.top) * scale_y)
        };
    }

    function start_drawing(e) {
        is_drawing = true;
        const coords = get_canvas_coordinates(e);
        last_x = coords.x;
        last_y = coords.y;

        if (current_tool === 'bucket') {
            flood_fill(coords.x, coords.y);
        }
        else if (current_tool === 'marquee') {
            selection = { x: coords.x, y: coords.y, width: 0, height: 0 };
        }
        else {
            draw(e);
        }
    }

    function draw_line(x0, y0, x1, y1) {
        const dx = Math.abs(x1 - x0);
        const dy = Math.abs(y1 - y0);
        const sx = x0 < x1 ? 1 : -1;
        const sy = y0 < y1 ? 1 : -1;
        let err = dx - dy;

        while (true) {
            if (current_tool === 'pencil') {
                ctx.fillStyle = selected_color;
                ctx.fillRect(x0, y0, 1, 1);
            }
            else if (current_tool === 'brush') {
                const half_size = Math.floor(brush_size / 2);
                for (let y = -half_size; y <= half_size; y++) {
                    for (let x = -half_size; x <= half_size; x++) {
                        ctx.fillStyle = selected_color;
                        ctx.fillRect(x0 + x, y0 + y, 1, 1);
                    }
                }
            }

            if (x0 === x1 && y0 === y1) break;
            const e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    function draw(e) {
        if (!is_drawing) return;
        const coords = get_canvas_coordinates(e);

        if (current_tool === 'pencil' || current_tool === 'brush') {
            draw_line(last_x, last_y, coords.x, coords.y);
            last_x = coords.x;
            last_y = coords.y;
        }
        else if (current_tool === 'marquee') {
            // Update selection rectangle
            selection.width = coords.x - selection.x;
            selection.height = coords.y - selection.y;
            redraw_canvas();
            draw_selection();
        }
    }

    function stop_drawing() {
        is_drawing = false;
        if (current_tool === 'marquee' && selection) {
            // Normalize selection coordinates
            if (selection.width < 0) {
                selection.x += selection.width;
                selection.width = -selection.width;
            }
            if (selection.height < 0) {
                selection.y += selection.height;
                selection.height = -selection.height;
            }
        }
    }

    function handle_mouse_out(e) {
        // Only update last_x/last_y if we're still drawing
        if (is_drawing) {
            const coords = get_canvas_coordinates(e);
            last_x = coords.x;
            last_y = coords.y;
        }
    }

    function flood_fill(x, y) {
        const image_data = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const target_color = get_pixel_color(image_data, x, y);
        const fill_color = hex_to_rgb(selected_color);

        if (colors_match(target_color, fill_color)) return;

        const stack = [{x, y}];
        const visited = new Set();

        while (stack.length > 0) {
            const {x, y} = stack.pop();
            const key = `${x},${y}`;

            if (visited.has(key)) continue;
            visited.add(key);

            if (x < 0 || x >= canvas.width || y < 0 || y >= canvas.height) continue;

            const current_color = get_pixel_color(image_data, x, y);
            if (!colors_match(current_color, target_color)) continue;

            set_pixel_color(image_data, x, y, fill_color);

            stack.push({x: x + 1, y});
            stack.push({x: x - 1, y});
            stack.push({x, y: y + 1});
            stack.push({x, y: y - 1});
        }

        ctx.putImageData(image_data, 0, 0);
    }

    function draw_selection() {
        if (!selection) return;
        ctx.strokeStyle = '#000';
        ctx.lineWidth = 1;
        ctx.setLineDash([5, 5]);
        ctx.strokeRect(selection.x, selection.y, selection.width, selection.height);
        ctx.setLineDash([]);
    }

    function redraw_canvas() {
        // Redraw the canvas content
        const image_data = ctx.getImageData(0, 0, canvas.width, canvas.height);
        ctx.putImageData(image_data, 0, 0);
    }

    // Clipboard functions
    copy_button.addEventListener('click', () => {
        if (!selection) return;
        const image_data = ctx.getImageData(selection.x, selection.y, selection.width, selection.height);
        clipboard = image_data;
    });

    paste_button.addEventListener('click', () => {
        if (!clipboard) return;
        ctx.putImageData(clipboard, selection ? selection.x : 0, selection ? selection.y : 0);
    });

    cut_button.addEventListener('click', () => {
        if (!selection) return;
        const image_data = ctx.getImageData(selection.x, selection.y, selection.width, selection.height);
        clipboard = image_data;
        ctx.fillStyle = '#004058';  // NES color 0x0F
        ctx.fillRect(selection.x, selection.y, selection.width, selection.height);
    });

    // Clear canvas with NES color 0x0F
    clear_button.addEventListener('click', () => {
        ctx.fillStyle = '#004058';  // NES color 0x0F
        ctx.fillRect(0, 0, canvas.width, canvas.height);
    });

    // Load image from localStorage if available
    const stored_image = localStorage.getItem('canvasImage');
    if (stored_image) {
        const img = new Image();
        img.onload = () => {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
        };
        img.src = stored_image;
    }

    // Update localStorage on canvas changes
    function update_local_storage() {
        const data_url = canvas.toDataURL('image/png');
        localStorage.setItem('canvasImage', data_url);
    }

    // Update localStorage on drawing actions
    canvas.addEventListener('mouseup', update_local_storage);
    canvas.addEventListener('mouseout', update_local_storage);

    // Update localStorage when clearing the canvas
    clear_button.addEventListener('click', () => {
        ctx.fillStyle = '#004058';  // NES color 0x0F
        ctx.fillRect(0, 0, canvas.width, canvas.height);
        update_local_storage();
    });

    // Load image onto canvas
    const load_image_input = document.getElementById('loadImage');

    // Update localStorage on loading an image
    load_image_input.addEventListener('change', (e) => {
        const file = e.target.files[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (event) => {
            const img = new Image();
            img.onload = () => {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
                update_local_storage();
            };
            img.src = event.target.result;
        };
        reader.readAsDataURL(file);
    });

    // Enable drag-and-drop image loading
    canvas.addEventListener('dragover', (e) => {
        e.preventDefault();
        e.dataTransfer.dropEffect = 'copy';
    });

    // Update localStorage on drag-and-drop
    canvas.addEventListener('drop', (e) => {
        e.preventDefault();
        const file = e.dataTransfer.files[0];
        if (!file || !file.type.startsWith('image/')) return;

        const reader = new FileReader();
        reader.onload = (event) => {
            const img = new Image();
            img.onload = () => {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
                update_local_storage();
            };
            img.src = event.target.result;
        };
        reader.readAsDataURL(file);
    });

    // Add back the save button functionality for downloading the image
    save_button.addEventListener('click', () => {
        const link = document.createElement('a');
        link.download = 'nes-drawing.png';
        link.href = canvas.toDataURL('image/png');
        link.click();
    });

    // Initialize grid
    draw_grid();

    // Add keyboard shortcuts for tools and grid toggle
    document.addEventListener('keydown', (e) => {
        switch (e.key.toLowerCase()) {
            case 'p':
                set_active_tool('pencil');
                break;
            case 'b':
                set_active_tool('brush');
                break;
            case 'f':
                set_active_tool('bucket');
                break;
            case 'g':
                show_grid = !show_grid;
                draw_grid();
                break;
        }
    });

    // Reduce colors to 4 max
    reduce_colors_button.addEventListener('click', () => {
        const image_data = ctx.getImageData(0, 0, canvas.width, canvas.height);
        
        // Process each grid cell (48x48 pixels)
        for (let grid_y = 0; grid_y < canvas.height; grid_y += 48) {
            for (let grid_x = 0; grid_x < canvas.width; grid_x += 48) {
                const colors = new Set();
                const color_counts = new Map();
                
                // Collect colors and their frequencies within this grid cell
                for (let y = grid_y; y < Math.min(grid_y + 48, canvas.height); y++) {
                    for (let x = grid_x; x < Math.min(grid_x + 48, canvas.width); x++) {
                        const color = get_pixel_color(image_data, x, y);
                        const color_str = `${color.r},${color.g},${color.b}`;
                        colors.add(color_str);
                        color_counts.set(color_str, (color_counts.get(color_str) || 0) + 1);
                    }
                }
                
                // If we already have 4 or fewer colors in this cell, skip it
                if (colors.size <= 4) continue;
                
                // Sort colors by frequency and take top 4
                const sorted_colors = Array.from(color_counts.entries())
                    .sort((a, b) => b[1] - a[1])
                    .slice(0, 4);
                
                // Create a map of old colors to new colors for this cell
                const color_map = new Map();
                for (let [color_str, _] of color_counts.entries()) {
                    if (!sorted_colors.some(([c, _]) => c === color_str)) {
                        // Find the closest color from the top 4
                        const [r, g, b] = color_str.split(',').map(Number);
                        let min_dist = Infinity;
                        let closest_color = null;
                        
                        for (let [target_str, _] of sorted_colors) {
                            const [tr, tg, tb] = target_str.split(',').map(Number);
                            const dist = Math.sqrt(
                                Math.pow(r - tr, 2) +
                                Math.pow(g - tg, 2) +
                                Math.pow(b - tb, 2)
                            );
                            
                            if (dist < min_dist) {
                                min_dist = dist;
                                closest_color = target_str;
                            }
                        }
                        
                        color_map.set(color_str, closest_color);
                    }
                }
                
                // Apply the color reduction within this grid cell
                for (let y = grid_y; y < Math.min(grid_y + 48, canvas.height); y++) {
                    for (let x = grid_x; x < Math.min(grid_x + 48, canvas.width); x++) {
                        const color = get_pixel_color(image_data, x, y);
                        const color_str = `${color.r},${color.g},${color.b}`;
                        
                        if (color_map.has(color_str)) {
                            const [r, g, b] = color_map.get(color_str).split(',').map(Number);
                            set_pixel_color(image_data, x, y, { r, g, b });
                        }
                    }
                }
            }
        }
        
        ctx.putImageData(image_data, 0, 0);
        update_local_storage();
    });
}); 