const fs = require("fs");
const path = require("path");

const BASE_DIR = "libcivaMSFS24";
const output = fs.openSync(path.join(BASE_DIR, "stack-analysis.su"), "w");
const THRESHOLD = parseInt(process.argv[2] ?? 1024);

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
  }

  fs.writeSync(
    output,
    `${_function.cost}\t${_function.type}\t${_function.name}\n`,
    null,
    { encoding: "utf8" }
  );
});

fs.closeSync(output);
