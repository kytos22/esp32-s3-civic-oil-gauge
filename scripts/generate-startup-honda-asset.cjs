#!/usr/bin/env node
"use strict";

const fs = require("fs");
const path = require("path");
const zlib = require("zlib");

const root = path.resolve(__dirname, "..");
const source = path.join(root, "docs/design/assets/startup-honda.png");
const output = path.join(root, "src/icons/startup_honda.c");
const png = fs.readFileSync(source);
const signature = "89504e470d0a1a0a";
if (png.subarray(0, 8).toString("hex") !== signature) {
  throw new Error("invalid PNG signature");
}

let cursor = 8;
let width = 0;
let height = 0;
let bitDepth = 0;
let colorType = 0;
const idat = [];
while (cursor < png.length) {
  const length = png.readUInt32BE(cursor);
  const type = png.subarray(cursor + 4, cursor + 8).toString("ascii");
  const data = png.subarray(cursor + 8, cursor + 8 + length);
  cursor += 12 + length;
  if (type === "IHDR") {
    width = data.readUInt32BE(0);
    height = data.readUInt32BE(4);
    bitDepth = data[8];
    colorType = data[9];
    if (data[12] !== 0) throw new Error("interlaced PNG is unsupported");
  } else if (type === "IDAT") {
    idat.push(data);
  } else if (type === "IEND") {
    break;
  }
}
if (width !== 320 || height !== 215 || bitDepth !== 16 || colorType !== 6) {
  throw new Error(`unexpected PNG format: ${width}x${height}, depth=${bitDepth}, type=${colorType}`);
}

const packed = zlib.inflateSync(Buffer.concat(idat));
const bytesPerPixel = 8;
const stride = width * bytesPerPixel;
const image = Buffer.alloc(stride * height);
const paeth = (a, b, c) => {
  const p = a + b - c;
  const pa = Math.abs(p - a);
  const pb = Math.abs(p - b);
  const pc = Math.abs(p - c);
  return pa <= pb && pa <= pc ? a : (pb <= pc ? b : c);
};
for (let y = 0; y < height; y += 1) {
  const packedRow = y * (stride + 1);
  const filter = packed[packedRow];
  for (let x = 0; x < stride; x += 1) {
    const raw = packed[packedRow + 1 + x];
    const left = x >= bytesPerPixel ? image[y * stride + x - bytesPerPixel] : 0;
    const above = y > 0 ? image[(y - 1) * stride + x] : 0;
    const upperLeft = y > 0 && x >= bytesPerPixel
      ? image[(y - 1) * stride + x - bytesPerPixel] : 0;
    let value = raw;
    if (filter === 1) value += left;
    else if (filter === 2) value += above;
    else if (filter === 3) value += Math.floor((left + above) / 2);
    else if (filter === 4) value += paeth(left, above, upperLeft);
    else if (filter !== 0) throw new Error(`unsupported PNG filter ${filter}`);
    image[y * stride + x] = value & 0xff;
  }
}

const colors = [];
const alpha = [];
for (let offset = 0; offset < image.length; offset += 8) {
  const red = image[offset];
  const green = image[offset + 2];
  const blue = image[offset + 4];
  const rgb565 = ((red & 0xf8) << 8) | ((green & 0xfc) << 3) | (blue >> 3);
  colors.push(rgb565 & 0xff, rgb565 >> 8);
  alpha.push(image[offset + 6]);
}
const bytes = colors.concat(alpha);

const rows = [];
for (let offset = 0; offset < bytes.length; offset += 12) {
  rows.push("    " + bytes.slice(offset, offset + 12)
    .map(value => `0x${value.toString(16).padStart(2, "0")}`).join(", ") + ",");
}

fs.writeFileSync(output,
  '#include "lvgl.h"\n\n' +
  'static const uint8_t startup_honda_logo_map[] = {\n' + rows.join("\n") + '\n};\n\n' +
  'const lv_image_dsc_t startup_honda_logo = {\n' +
  '    .header = {\n' +
  '        .magic = LV_IMAGE_HEADER_MAGIC,\n' +
  '        .cf = LV_COLOR_FORMAT_RGB565A8,\n' +
  '        .flags = 0,\n' +
  '        .w = 320,\n' +
  '        .h = 215,\n' +
  '        .stride = 640,\n' +
  '        .reserved_2 = 0,\n' +
  '    },\n' +
  '    .data_size = sizeof(startup_honda_logo_map),\n' +
  '    .data = startup_honda_logo_map,\n' +
  '    .reserved = NULL,\n' +
  '};\n');
