#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const {PNG} = require('pngjs');

const root = path.resolve(__dirname, '..');
const inputs = {
  temperature: process.argv[2],
  pressure: process.argv[3],
};
if (!inputs.temperature || !inputs.pressure) {
  throw new Error('usage: generate-oil-icon-assets.cjs TEMP.png PRESSURE.png');
}

const width = 92;
const height = 72;

function makeMask(input, name) {
  const image = PNG.sync.read(fs.readFileSync(input));
  const {data} = image;
  let left = image.width;
  let top = image.height;
  let right = -1;
  let bottom = -1;
  const alpha = Buffer.alloc(image.width * image.height);
  for (let y = 0; y < image.height; y += 1) {
    for (let x = 0; x < image.width; x += 1) {
      const source = (y * image.width + x) * 4;
      const value = Math.max(data[source], data[source + 1], data[source + 2]);
      const opacity = Math.round(value * data[source + 3] / 255);
      const mask = opacity < 24 ? 0 : opacity;
      alpha[y * image.width + x] = mask;
      if (mask > 24) {
        left = Math.min(left, x);
        top = Math.min(top, y);
        right = Math.max(right, x);
        bottom = Math.max(bottom, y);
      }
    }
  }
  if (right < left || bottom < top) throw new Error(`${input}: empty icon`);

  const cropWidth = right - left + 1;
  const cropHeight = bottom - top + 1;
  const scale = Math.min(width / cropWidth, height / cropHeight);
  const drawWidth = Math.max(1, Math.round(cropWidth * scale));
  const drawHeight = Math.max(1, Math.round(cropHeight * scale));
  const offsetX = Math.floor((width - drawWidth) / 2);
  const offsetY = Math.floor((height - drawHeight) / 2);
  const cropped = Buffer.alloc(width * height);
  for (let y = 0; y < drawHeight; y += 1) {
    for (let x = 0; x < drawWidth; x += 1) {
      const sourceX = left + Math.min(cropWidth - 1, Math.floor(x / scale));
      const sourceY = top + Math.min(cropHeight - 1, Math.floor(y / scale));
      cropped[(y + offsetY) * width + x + offsetX] =
          alpha[sourceY * image.width + sourceX];
    }
  }

  const rgba = Buffer.alloc(width * height * 4);
  for (let index = 0; index < cropped.length; index += 1) {
    rgba[index * 4] = 255;
    rgba[index * 4 + 1] = 255;
    rgba[index * 4 + 2] = 255;
    rgba[index * 4 + 3] = cropped[index];
  }
  const preview = new PNG({width, height});
  rgba.copy(preview.data);
  fs.writeFileSync(
      path.join(root, 'docs/design/assets', `oil-${name}-icon-mask.png`),
      PNG.sync.write(preview));
  return cropped;
}

function bytes(name, data) {
  const rows = [];
  for (let offset = 0; offset < data.length; offset += 16) {
    rows.push(`    ${Array.from(data.subarray(offset, offset + 16), value =>
      `0x${value.toString(16).padStart(2, '0')}`).join(', ')},`);
  }
  return `const uint8_t ${name}_map[] = {\n${rows.join('\n')}\n};`;
}

(() => {
  fs.mkdirSync(path.join(root, 'src/icons'), {recursive: true});
  fs.mkdirSync(path.join(root, 'docs/design/assets'), {recursive: true});
  const temperature = makeMask(inputs.temperature, 'temperature');
  const pressure = makeMask(inputs.pressure, 'pressure');
  const header = `#pragma once\n\n#include "lvgl.h"\n\nLV_IMAGE_DECLARE(oil_temperature_icon);\nLV_IMAGE_DECLARE(oil_pressure_icon);\n`;
  const descriptor = name => `const lv_image_dsc_t ${name} = {\n  .header = {\n    .magic = LV_IMAGE_HEADER_MAGIC,\n    .cf = LV_COLOR_FORMAT_A8,\n    .flags = 0,\n    .w = ${width},\n    .h = ${height},\n    .stride = ${width},\n    .reserved_2 = 0,\n  },\n  .data_size = sizeof(${name}_map),\n  .data = ${name}_map,\n  .reserved = NULL,\n};`;
  const source = `#include "oil_icon_assets.h"\n\n#include <stdint.h>\n\n${bytes('oil_temperature_icon', temperature)}\n\n${descriptor('oil_temperature_icon')}\n\n${bytes('oil_pressure_icon', pressure)}\n\n${descriptor('oil_pressure_icon')}\n`;
  fs.writeFileSync(path.join(root, 'src/icons/oil_icon_assets.h'), header);
  fs.writeFileSync(path.join(root, 'src/icons/oil_icon_assets.c'), source);
})();
