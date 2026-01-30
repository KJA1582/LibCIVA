const fs = require("fs");
const path = require("path");

const BASE_DIRS = [
  "libciva",
  "Examples/civa24"
];

let THRESHOLD = parseInt(process.argv.at(-1) ?? 1024);
THRESHOLD = isNaN(THRESHOLD) ? 1024 : THRESHOLD;
console.log(`Threshold at ${THRESHOLD} bytes`);
let LEVEL = process.argv.includes("--level") ? process.argv[process.argv.findIndex((p) => p === "--level") +1] : undefined;
if (LEVEL !== "info") LEVEL = undefined;
console.log(`Log level ${LEVEL ?? "default"}`);

BASE_DIRS.forEach(BASE_DIR => {
  console.log(`Analysis for ${BASE_DIR}`)

  const output = fs.openSync(path.join(BASE_DIR, "stack-analysis.log"), "w");

  let functions = [];

  let files = fs.readdirSync(BASE_DIR);
  files = files.filter((file) => file.endsWith(".su"));
  files.forEach((file) => {
    const content = fs.readFileSync(path.join(BASE_DIR, file), {
      encoding: "utf8",
    });

    const _functions = content.split("\n");
    _functions.forEach((_function) => {
      const fields = _function.split("\t");

      if (fields.length != 3) return;

      functions.push({
        name: fields[0],
        cost: parseInt(fields[1]),
        type: fields[2],
      });
    });
  
    fs.unlinkSync(path.join(BASE_DIR, file));
  });

  functions = functions.sort((a, b) => b.cost - a.cost);
  functions.forEach((_function) => {
    if (_function.cost === THRESHOLD) {
      console.warn(
        `\x1b[33mFunction \x1b[1m${_function.name}\x1b[0m \x1b[33mat threshold (\x1b[1m${_function.cost}\x1b[0m\x1b[33m)\x1b[0m`
      );
    } else if (_function.cost >= THRESHOLD) {
      console.error(
        `\x1b[31mFunction \x1b[1m${_function.name}\x1b[0m \x1b[31mexceeds threshold (\x1b[1m${_function.cost}\x1b[0m\x1b[31m)\x1b[0m`
      );
    } else if (LEVEL === "info") {
      console.error(
        `\x1b[32mFunction \x1b[1m${_function.name}\x1b[0m \x1b[32mbelow threshold (\x1b[1m${_function.cost}\x1b[0m\x1b[32m)\x1b[0m`
      );
    }

    fs.writeSync(
      output,
      `${_function.cost}\t${_function.type}\t${_function.name}\n`,
      null,
      { encoding: "utf8" }
    );
  });

  fs.closeSync(output);
});