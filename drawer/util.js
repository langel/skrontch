// Utility functions for pixel manipulation and color conversion

function get_pixel_color(image_data, x, y) {
    const i = (y * image_data.width + x) * 4;
    return {
        r: image_data.data[i],
        g: image_data.data[i + 1],
        b: image_data.data[i + 2],
        a: image_data.data[i + 3]
    };
}

function set_pixel_color(image_data, x, y, color) {
    const i = (y * image_data.width + x) * 4;
    image_data.data[i] = color.r;
    image_data.data[i + 1] = color.g;
    image_data.data[i + 2] = color.b;
    image_data.data[i + 3] = 255;
}

function colors_match(c1, c2) {
    return c1.r === c2.r && c1.g === c2.g && c1.b === c2.b;
}

function hex_to_rgb(hex) {
    const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
        r: parseInt(result[1], 16),
        g: parseInt(result[2], 16),
        b: parseInt(result[3], 16)
    } : null;
} 