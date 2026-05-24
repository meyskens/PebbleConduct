#!/usr/bin/env node

/**
 * Pebble Icon Converter
 * 
 * Converts SVG icons to PNG format for use in Pebble watch apps.
 * Uses the 'sharp' image processing library.
 * 
 * Usage:
 *   node convert-icon.js <input.svg> [output.png] [size]
 * 
 * Examples:
 *   node convert-icon.js train-icon.svg
 *   node convert-icon.js train-icon.svg resources/images/train.png
 *   node convert-icon.js train-icon.svg resources/images/train.png 24
 */

const sharp = require('sharp');
const fs = require('fs');
const path = require('path');

function showUsage() {
  console.log(`
Pebble Icon Converter

Converts SVG icons to PNG format for Pebble watch apps.

Usage:
  node convert-icon.js <input.svg> [output.png] [size]

Arguments:
  input.svg    - Path to input SVG file (required)
  output.png   - Path to output PNG file (optional, defaults to same name as input)
  size         - Icon size in pixels (optional, defaults to 18)

Examples:
  node convert-icon.js train-icon.svg
  node convert-icon.js train-icon.svg resources/images/train.png
  node convert-icon.js train-icon.svg resources/images/train.png 24
`);
}

function main() {
  const args = process.argv.slice(2);

  if (args.length === 0 || args.includes('--help') || args.includes('-h')) {
    showUsage();
    process.exit(0);
  }

  const inputPath = args[0];
  
  // Default output path: same directory, same name but .png extension
  const defaultOutputPath = inputPath.replace(/\.svg$/i, '.png');
  const outputPath = args[1] || defaultOutputPath;
  
  // Default size: 18px (good for Pebble text row height)
  const size = parseInt(args[2], 10) || 18;

  // Validate input file exists
  if (!fs.existsSync(inputPath)) {
    console.error(`Error: Input file not found: ${inputPath}`);
    process.exit(1);
  }

  // Validate input is SVG
  if (!inputPath.toLowerCase().endsWith('.svg')) {
    console.error(`Error: Input file must be an SVG file`);
    process.exit(1);
  }

  // Ensure output directory exists
  const outputDir = path.dirname(outputPath);
  if (!fs.existsSync(outputDir)) {
    console.log(`Creating directory: ${outputDir}`);
    fs.mkdirSync(outputDir, { recursive: true });
  }

  // Read and convert
  try {
    const svgBuffer = fs.readFileSync(inputPath);
    
    sharp(svgBuffer)
      .resize(size, size, { 
        fit: 'contain', 
        background: { r: 0, g: 0, b: 0, alpha: 0 } 
      })
      .png()
      .toFile(outputPath)
      .then(() => {
        console.log(`✓ Converted: ${inputPath} → ${outputPath} (${size}x${size})`);
      })
      .catch(err => {
        console.error(`Error converting image: ${err.message}`);
        process.exit(1);
      });
  } catch (err) {
    console.error(`Error reading file: ${err.message}`);
    process.exit(1);
  }
}

main();
