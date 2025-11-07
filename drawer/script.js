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

    // Initialize current_palettes from localStorage
    let current_palettes = [];
    try {
        const saved_palettes = localStorage.getItem('reducedPalettes');
        if (saved_palettes) {
            current_palettes = JSON.parse(saved_palettes);
        }
    } catch (e) {
        console.error('Error loading palettes from localStorage:', e);
        current_palettes = [];
    }

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

    // Reduce colors to 4 palettes with shared first color
    reduce_colors_button.addEventListener('click', () => {
        const loading_overlay = document.getElementById('loadingOverlay');
        loading_overlay.style.display = 'flex';

        // Use setTimeout to allow the UI to update before starting the heavy processing
        setTimeout(() => {
            const image_data = ctx.getImageData(0, 0, canvas.width, canvas.height);
            const color_counts = new Map();
            
            // First pass: collect all colors and their frequencies, converting to closest NES colors
            for (let y = 0; y < canvas.height; y++) {
                for (let x = 0; x < canvas.width; x++) {
                    const color = get_pixel_color(image_data, x, y);
                    // Find closest NES color
                    const nes_color = find_closest_nes_color(color);
                    const color_str = `${nes_color.r},${nes_color.g},${nes_color.b}`;
                    color_counts.set(color_str, (color_counts.get(color_str) || 0) + 1);
                }
            }
            
            // Find the most common NES color (this will be our shared color)
            const sorted_colors = Array.from(color_counts.entries())
                .sort((a, b) => b[1] - a[1]);
            const shared_color = sorted_colors[0][0];
            
            // Remove the shared color from the counts
            color_counts.delete(shared_color);
            
            // Sort remaining NES colors by frequency
            const remaining_colors = Array.from(color_counts.entries())
                .sort((a, b) => b[1] - a[1]);
            
            // Create 4 palettes, each with the shared color and 3 unique NES colors
            current_palettes = [];
            for (let i = 0; i < 4; i++) {
                const palette = [shared_color];
                // Take 3 unique NES colors for this palette
                for (let j = 0; j < 3; j++) {
                    const color_index = i * 3 + j;
                    if (color_index < remaining_colors.length) {
                        palette.push(remaining_colors[color_index][0]);
                    } else {
                        // If we run out of unique colors, use the shared color
                        palette.push(shared_color);
                    }
                }
                current_palettes.push(palette);
            }

            // Update the canvas with the reduced NES colors
            const new_image_data = ctx.createImageData(canvas.width, canvas.height);
            for (let y = 0; y < canvas.height; y++) {
                for (let x = 0; x < canvas.width; x++) {
                    const color = get_pixel_color(image_data, x, y);
                    const nes_color = find_closest_nes_color(color);
                    const pixel_index = (y * canvas.width + x) * 4;
                    new_image_data.data[pixel_index] = nes_color.r;
                    new_image_data.data[pixel_index + 1] = nes_color.g;
                    new_image_data.data[pixel_index + 2] = nes_color.b;
                    new_image_data.data[pixel_index + 3] = 255;
                }
            }
            ctx.putImageData(new_image_data, 0, 0);

            // Save palettes to localStorage
            localStorage.setItem('reducedPalettes', JSON.stringify(current_palettes));

            // Display the generated palettes
            const palettes_container = document.getElementById('generatedPalettes');
            palettes_container.innerHTML = '';
            
            current_palettes.forEach((palette, index) => {
                const palette_display = document.createElement('div');
                palette_display.className = 'palette-display';
                
                const title = document.createElement('div');
                title.className = 'palette-title';
                title.textContent = `Palette ${index + 1}`;
                palette_display.appendChild(title);
                
                const colors_container = document.createElement('div');
                colors_container.className = 'palette-colors';
                
                palette.forEach((color_str, color_index) => {
                    const color_div = document.createElement('div');
                    color_div.className = 'palette-color';
                    if (color_index === 0) {
                        color_div.classList.add('shared-color');
                    }
                    
                    const [r, g, b] = color_str.split(',').map(Number);
                    color_div.style.backgroundColor = `rgb(${r}, ${g}, ${b})`;
                    
                    // Add tooltip with NES palette color index
                    const nes_color_index = nes_palette.findIndex(palette_color => 
                        palette_color.r === r && 
                        palette_color.g === g && 
                        palette_color.b === b
                    );
                    if (nes_color_index !== -1) {
                        const color_id = nes_color_index.toString(16).toUpperCase().padStart(2, '0');
                        color_div.title = `Color $${color_id}`;
                    }
                    
                    colors_container.appendChild(color_div);
                });
                
                palette_display.appendChild(colors_container);
                palettes_container.appendChild(palette_display);
            });

            // Hide the loading overlay
            loading_overlay.style.display = 'none';
        }, 100);
    });

    // Function to find the closest NES color
    function find_closest_nes_color(color) {
        let min_distance = Infinity;
        let closest_color = null;
        
        for (let i = 0; i < nes_palette.length; i++) {
            const nes_color = hex_to_rgb(nes_palette[i]);
            const distance = color_distance(color, nes_color);
            if (distance < min_distance) {
                min_distance = distance;
                closest_color = nes_color;
            }
        }
        
        return closest_color;
    }
    
    // Function to calculate color distance (using Euclidean distance in RGB space)
    function color_distance(color1, color2) {
        const dr = color1.r - color2.r;
        const dg = color1.g - color2.g;
        const db = color1.b - color2.b;
        return dr * dr + dg * dg + db * db;
    }

    // Export NES data
    const export_nes_button = document.getElementById('exportNESData');
    const chr_link = document.getElementById('chrLink');
    const nametable_link = document.getElementById('nametableLink');
    const export_links = document.getElementById('exportLinks');
    const chr_preview = document.getElementById('chrPreview');
    const reconstructed_preview = document.getElementById('reconstructedPreview');
    const chr_preview_ctx = chr_preview.getContext('2d');
    const reconstructed_preview_ctx = reconstructed_preview.getContext('2d');

    export_nes_button.addEventListener('click', () => {
        if (!current_palettes) {
            alert('Please reduce colors first before exporting NES data');
            return;
        }

        // Show loading overlay
        const loading_overlay = document.getElementById('loadingOverlay');
        loading_overlay.style.display = 'flex';
        loading_overlay.querySelector('.loading-text').textContent = 'Generating NES data...';

        // Use setTimeout to allow the UI to update before starting the heavy processing
        setTimeout(() => {
            // Get the current canvas data
            const image_data = ctx.getImageData(0, 0, canvas.width, canvas.height);
            
            // Create a map of unique 8x8 tiles
            const tiles = new Map();
            const tile_map = new Uint8Array(32 * 30); // 32x30 tile map
            const attribute_table = new Uint8Array(64); // 8x8 attribute table (2x2 tile regions)
            
            // Process the image in 8x8 pixel tiles
            for (let tile_y = 0; tile_y < 30; tile_y++) {
                for (let tile_x = 0; tile_x < 32; tile_x++) {
                    // Extract the 8x8 tile
                    const tile_data = new Uint8Array(64); // 8x8 pixels
                    for (let y = 0; y < 8; y++) {
                        for (let x = 0; x < 8; x++) {
                            const pixel_x = tile_x * 8 + x;
                            const pixel_y = tile_y * 8 + y;
                            const color = get_pixel_color(image_data, pixel_x, pixel_y);
                            
                            // Find which palette this color belongs to
                            let palette_index = 0;
                            for (let i = 0; i < current_palettes.length; i++) {
                                const palette = current_palettes[i];
                                if (palette.some(c => {
                                    const [r, g, b] = c.split(',').map(Number);
                                    return r === color.r && g === color.g && b === color.b;
                                })) {
                                    palette_index = i;
                                    break;
                                }
                            }
                            
                            // Store the color index in the tile (0-3)
                            const color_index = current_palettes[palette_index].findIndex(c => {
                                const [r, g, b] = c.split(',').map(Number);
                                return r === color.r && g === color.g && b === color.b;
                            });
                            tile_data[y * 8 + x] = color_index;
                            
                            // Update attribute table (2x2 tile regions)
                            if (x % 2 === 0 && y % 2 === 0) {
                                const attr_x = Math.floor(tile_x / 2);
                                const attr_y = Math.floor(tile_y / 2);
                                const attr_index = attr_y * 8 + attr_x;
                                attribute_table[attr_index] = palette_index;
                            }
                        }
                    }
                    
                    // Convert tile data to NES format (2 bit planes)
                    const tile_bytes = new Uint8Array(16);
                    for (let y = 0; y < 8; y++) {
                        let low_byte = 0;
                        let high_byte = 0;
                        for (let x = 0; x < 8; x++) {
                            const color_index = tile_data[y * 8 + x];
                            low_byte |= ((color_index >> 0) & 1) << (7 - x);
                            high_byte |= ((color_index >> 1) & 1) << (7 - x);
                        }
                        tile_bytes[y] = low_byte;
                        tile_bytes[y + 8] = high_byte;
                    }
                    
                    // Check if this tile already exists
                    let tile_index = -1;
                    for (const [index, existing_tile] of tiles.entries()) {
                        let match = true;
                        for (let i = 0; i < 16; i++) {
                            if (existing_tile[i] !== tile_bytes[i]) {
                                match = false;
                                break;
                            }
                        }
                        if (match) {
                            tile_index = index;
                            break;
                        }
                    }
                    
                    // If tile doesn't exist, add it
                    if (tile_index === -1) {
                        tile_index = tiles.size;
                        tiles.set(tile_index, tile_bytes);
                    }
                    
                    // Store tile index in tile map
                    tile_map[tile_y * 32 + tile_x] = tile_index;
                }
            }
            
            // Create CHR data (all unique tiles)
            const chr_data = new Uint8Array(tiles.size * 16);
            for (const [index, tile] of tiles.entries()) {
                chr_data.set(tile, index * 16);
            }
            
            // Create name table data (tile map + attribute table)
            const name_table_data = new Uint8Array(1024); // 1KB name table
            name_table_data.set(tile_map, 0); // First 960 bytes are tile map
            name_table_data.set(attribute_table, 960); // Last 64 bytes are attribute table
            
            // Create Blob URLs for the data
            const chr_blob = new Blob([chr_data], { type: 'application/octet-stream' });
            const name_table_blob = new Blob([name_table_data], { type: 'application/octet-stream' });
            
            // Update the links with download attributes
            chr_link.href = URL.createObjectURL(chr_blob);
            chr_link.download = 'pattern_table.chr';
            nametable_link.href = URL.createObjectURL(name_table_blob);
            nametable_link.download = 'name_table.nam';
            
            // Render CHR preview
            chr_preview_ctx.clearRect(0, 0, chr_preview.width, chr_preview.height);
            const tiles_per_row = Math.floor(chr_preview.width / 8);
            
            // Ensure we have valid palettes to work with
            if (!current_palettes || current_palettes.length === 0) {
                console.error('No palettes available for CHR preview');
                return;
            }
            
            // Create a default palette as fallback
            const default_palette = ['0,0,0', '0,0,0', '0,0,0', '0,0,0'];
            
            for (const [index, tile] of tiles.entries()) {
                const tile_x = (index % tiles_per_row) * 8;
                const tile_y = Math.floor(index / tiles_per_row) * 8;
                
                // Get the palette for this tile - use a default palette if we can't determine it
                let palette_index = 0;
                let palette = current_palettes[0] || default_palette;
                
                // Try to find the palette used by this tile in the original image
                for (let tile_y_img = 0; tile_y_img < 30; tile_y_img++) {
                    for (let tile_x_img = 0; tile_x_img < 32; tile_x_img++) {
                        if (tile_map[tile_y_img * 32 + tile_x_img] === index) {
                            const attr_x = Math.floor(tile_x_img / 2);
                            const attr_y = Math.floor(tile_y_img / 2);
                            const attr_index = attr_y * 8 + attr_x;
                            if (attr_index < attribute_table.length) {
                                palette_index = attribute_table[attr_index];
                                if (palette_index < current_palettes.length) {
                                    palette = current_palettes[palette_index];
                                }
                            }
                            break;
                        }
                    }
                    if (palette_index !== 0) break;
                }
                
                // Ensure palette has all required colors
                if (!palette || palette.length < 4) {
                    palette = default_palette;
                }
                
                // Render the tile
                for (let y = 0; y < 8; y++) {
                    for (let x = 0; x < 8; x++) {
                        const low_bit = (tile[y] >> (7 - x)) & 1;
                        const high_bit = (tile[y + 8] >> (7 - x)) & 1;
                        const color_index = (high_bit << 1) | low_bit;
                        
                        // Ensure color_index is within bounds
                        const safe_color_index = Math.min(color_index, palette.length - 1);
                        const [r, g, b] = palette[safe_color_index].split(',').map(Number);
                        chr_preview_ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
                        chr_preview_ctx.fillRect(tile_x + x, tile_y + y, 1, 1);
                    }
                }
            }
            
            // Render reconstructed image
            reconstructed_preview_ctx.clearRect(0, 0, reconstructed_preview.width, reconstructed_preview.height);
            
            // Ensure we have valid palettes to work with
            if (!current_palettes || current_palettes.length === 0) {
                console.error('No palettes available for rendering');
                return;
            }
            
            // Create a default palette as fallback
            const default_palette_render = ['0,0,0', '0,0,0', '0,0,0', '0,0,0'];
            
            for (let tile_y = 0; tile_y < 30; tile_y++) {
                for (let tile_x = 0; tile_x < 32; tile_x++) {
                    const tile_index = tile_map[tile_y * 32 + tile_x];
                    const tile = tiles.get(tile_index);
                    
                    // Get the palette for this tile - use a default palette if we can't determine it
                    let palette_index = 0;
                    let palette = current_palettes[0] || default_palette_render;
                    
                    // Try to find the palette used by this tile in the original image
                    const attr_x = Math.floor(tile_x / 2);
                    const attr_y = Math.floor(tile_y / 2);
                    const attr_index = attr_y * 8 + attr_x;
                    
                    if (attr_index < attribute_table.length) {
                        palette_index = attribute_table[attr_index];
                        if (palette_index < current_palettes.length) {
                            palette = current_palettes[palette_index];
                        }
                    }
                    
                    // Ensure palette has all required colors
                    if (!palette || palette.length < 4) {
                        palette = default_palette_render;
                    }
                    
                    // Render the tile
                    for (let y = 0; y < 8; y++) {
                        for (let x = 0; x < 8; x++) {
                            const low_bit = (tile[y] >> (7 - x)) & 1;
                            const high_bit = (tile[y + 8] >> (7 - x)) & 1;
                            const color_index = (high_bit << 1) | low_bit;
                            
                            // Ensure color_index is within bounds
                            const safe_color_index = Math.min(color_index, palette.length - 1);
                            const [r, g, b] = palette[safe_color_index].split(',').map(Number);
                            reconstructed_preview_ctx.fillStyle = `rgb(${r}, ${g}, ${b})`;
                            reconstructed_preview_ctx.fillRect(tile_x * 8 + x, tile_y * 8 + y, 1, 1);
                        }
                    }
                }
            }
            
            // Show the export links section
            export_links.style.display = 'block';
            
            // Hide the loading overlay
            loading_overlay.style.display = 'none';
        }, 100);
    });

    // Add new session button functionality
    const new_session_button = document.getElementById('newSession');
    new_session_button.addEventListener('click', () => {
        if (confirm('Are you sure you want to start a new session? This will clear all saved data and reset the canvas.')) {
            // Clear localStorage
            localStorage.removeItem('canvasImage');
            localStorage.removeItem('selectedColor');
            localStorage.removeItem('selectedTool');
            localStorage.removeItem('brushSize');
            localStorage.removeItem('showGrid');
            localStorage.removeItem('reducedPalettes');
            
            // Reset canvas
            ctx.fillStyle = '#004058';  // NES color 0x0F
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            // Reset grid
            show_grid = false;
            draw_grid();
            
            // Reset palettes
            current_palettes = [];
            const palettes_container = document.getElementById('generatedPalettes');
            palettes_container.innerHTML = '';
            
            // Reset export links
            const export_links = document.getElementById('exportLinks');
            export_links.style.display = 'none';
            
            // Reset tool selection
            set_active_tool('pencil');
            
            // Reset brush size
            brush_size = 1;
            brush_size_input.value = brush_size;
            brush_size_value.textContent = brush_size;
            
            // Reset color selection to first color
            const first_swatch = document.querySelector('.color-swatch');
            if (first_swatch) {
                document.querySelectorAll('.color-swatch').forEach(s => s.classList.remove('selected'));
                first_swatch.classList.add('selected');
                selected_color = first_swatch.dataset.color;
            }
        }
    });
}); 